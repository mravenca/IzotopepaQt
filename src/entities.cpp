#include "entities.h"

#include <algorithm>
#include <cmath>

namespace {
constexpr double Gravity = 1900.0;
constexpr double WalkSpeed = 300.0;
constexpr double ClimbSpeed = 220.0;
constexpr double LadderSnapSpeed = 12.0;
}

Player::Player(const SpriteSheet *sheet)
    : sheet_(sheet)
{
}

void Player::reset(QPointF position, bool full)
{
    rect_ = QRectF(position.x(), position.y(), 46, 86);
    vel_ = {};
    left_ = right_ = up_ = down_ = false;
    onGround_ = onLadder_ = climbing_ = false;
    jumpHeld_ = false;
    inWater_ = false;
    waterDrag_ = 0.55;
    buoyancy_ = 0.72;
    surfaceFriction_ = 1.0;
    direction_ = 1;
    shootCd_ = invuln_ = anim_ = 0;
    muzzleFlash_ = 0;
    coyoteTime_ = jumpBuffer_ = 0;

    if (full) {
        health_ = 5;
        ammo_ = 20;
        score_ = 0;
        keys_.clear();
    }
}

void Player::update(
    double dt,
    const QVector<QRectF> &platforms,
    const QVector<QRectF> &oneWayPlatforms,
    const QVector<QRectF> &ladders,
    double worldWidth,
    bool ignoreOneWay)
{
    jumpBuffer_ = std::max(0.0, jumpBuffer_ - dt);
    coyoteTime_ = std::max(0.0, coyoteTime_ - dt);

    // The enlarged probe allows the player to grab a ladder while standing
    // immediately above or below it.
    const QRectF ladderProbe = rect_.adjusted(8.0, -12.0, -8.0, 12.0);

    QRectF activeLadder;
    onLadder_ = false;

    for (const QRectF &ladder : ladders) {
        if (ladderProbe.intersects(ladder)) {
            activeLadder = ladder;
            onLadder_ = true;
            break;
        }
    }

    const bool wantsVerticalMovement = up_ || down_;

    if (onLadder_ && wantsVerticalMovement) {
        climbing_ = true;
    } else if (!onLadder_) {
        climbing_ = false;
    }

    const int horizontal = (right_ ? 1 : 0) - (left_ ? 1 : 0);

    QVector<QRectF> collisionPlatforms = platforms;

    if (climbing_ && !activeLadder.isNull()) {
        // Keep the player centred on the ladder. This also makes climbing
        // narrow ladders reliable.
        const double targetLeft =
            activeLadder.center().x() - rect_.width() / 2.0;
        const double correction = std::clamp(
            targetLeft - rect_.left(),
            -LadderSnapSpeed,
            LadderSnapSpeed);
        rect_.translate(correction, 0.0);

        vel_.setX(0.0f);

        if (up_ == down_) {
            vel_.setY(0.0f);
        } else {
            vel_.setY(up_ ? -ClimbSpeed : ClimbSpeed);
        }

        // Platforms crossing the active ladder must not block the player.
        // Other platforms remain solid.
        const QRectF ladderPassage =
            activeLadder.adjusted(-2.0, -2.0, 2.0, 2.0);

        collisionPlatforms.erase(
            std::remove_if(
                collisionPlatforms.begin(),
                collisionPlatforms.end(),
                [&ladderPassage](const QRectF &platform) {
                    return platform.intersects(ladderPassage);
                }),
            collisionPlatforms.end());
    } else if (inWater_) {
        const float targetX = static_cast<float>(horizontal * 175.0);
        const float blend = static_cast<float>(
            std::min(1.0, dt * (4.0 + waterDrag_ * 6.0)));
        vel_.setX(vel_.x() + (targetX - vel_.x()) * blend);

        if (horizontal != 0) {
            direction_ = horizontal > 0 ? 1 : -1;
        }

        double verticalAcceleration = Gravity * (1.0 - buoyancy_) * 0.42;
        if (jumpHeld_ || up_) {
            verticalAcceleration -= 1150.0;
        }
        if (down_) {
            verticalAcceleration += 760.0;
        }

        vel_.setY(static_cast<float>(
            vel_.y() + verticalAcceleration * dt));

        const float dragFactor = static_cast<float>(
            std::exp(-waterDrag_ * 2.7 * dt));
        vel_ *= dragFactor;
        vel_.setY(std::clamp(vel_.y(), -280.0f, 330.0f));
    } else {
        if (surfaceFriction_ < 0.99) {
            const float targetX = static_cast<float>(horizontal * WalkSpeed);
            const float acceleration = static_cast<float>(
                (650.0 + 700.0 * surfaceFriction_) * dt);

            if (vel_.x() < targetX) {
                vel_.setX(std::min(targetX, vel_.x() + acceleration));
            } else if (vel_.x() > targetX) {
                vel_.setX(std::max(targetX, vel_.x() - acceleration));
            }

            if (horizontal == 0) {
                vel_.setX(vel_.x() * static_cast<float>(
                    std::exp(-surfaceFriction_ * 3.0 * dt)));
            }
        } else {
            vel_.setX(horizontal * WalkSpeed);
        }

        if (horizontal != 0) {
            direction_ = horizontal > 0 ? 1 : -1;
        }

        vel_.setY(std::min(
            static_cast<double>(vel_.y()) + Gravity * dt,
            1000.0));
    }

    const bool wasGrounded = onGround_;
    onGround_ = moveAndCollideOneWay(
        rect_,
        vel_,
        dt,
        collisionPlatforms,
        oneWayPlatforms,
        ignoreOneWay).onGround;

    if (onGround_) {
        coyoteTime_ = 0.10;
    } else if (wasGrounded && vel_.y() >= 0.0f) {
        coyoteTime_ = std::max(coyoteTime_, 0.10);
    }

    const bool canBufferedJump =
        onGround_ || coyoteTime_ > 0.0 || onLadder_ || climbing_;

    if (!inWater_ && jumpBuffer_ > 0.0 && canBufferedJump) {
        climbing_ = false;
        vel_.setY(-720.0f);
        onGround_ = false;
        coyoteTime_ = 0.0;
        jumpBuffer_ = 0.0;
    }

    if (rect_.left() < 0.0) {
        rect_.moveLeft(0.0);
    }

    if (rect_.right() > worldWidth) {
        rect_.moveRight(worldWidth);
    }

    shootCd_ = std::max(0.0, shootCd_ - dt);
    muzzleFlash_ = std::max(0.0, muzzleFlash_ - dt);
    invuln_ = std::max(0.0, invuln_ - dt);
    anim_ += dt;
}

void Player::setLeft(bool value) { left_ = value; }
void Player::setRight(bool value) { right_ = value; }
void Player::setUp(bool value) { up_ = value; }
void Player::setDown(bool value) { down_ = value; }

void Player::jump()
{
    jumpHeld_ = true;
    // Store the input briefly so a jump pressed just before landing is
    // consumed on the first legal frame.
    jumpBuffer_ = 0.12;
}

void Player::stopJump()
{
    jumpHeld_ = false;
    // Releasing jump while rising reduces upward speed, producing a short hop.
    // It does not affect falling, ladder movement, or knock-back.
    if (!inWater_ && !climbing_ && vel_.y() < -250.0f) {
        vel_.setY(vel_.y() * 0.45f);
    }
}

void Player::setEnvironment(
    bool inWater,
    double waterDrag,
    double buoyancy,
    double surfaceFriction)
{
    inWater_ = inWater;
    waterDrag_ = std::clamp(waterDrag, 0.0, 1.0);
    buoyancy_ = std::clamp(buoyancy, 0.0, 1.5);
    surfaceFriction_ = std::clamp(surfaceFriction, 0.01, 1.0);

    if (inWater_) {
        climbing_ = false;
        onLadder_ = false;
        coyoteTime_ = 0.0;
    }
}

bool Player::inWater() const
{
    return inWater_;
}

void Player::launch(const QVector2D &impulse)
{
    climbing_ = false;
    onGround_ = false;
    coyoteTime_ = 0.0;
    jumpBuffer_ = 0.0;
    vel_ = impulse;
}

QVector2D Player::velocity() const
{
    return vel_;
}

void Player::dropThroughOneWay()
{
    climbing_ = false;
    onGround_ = false;
    coyoteTime_ = 0.0;
    jumpBuffer_ = 0.0;
    vel_.setY(std::max(vel_.y(), 150.0f));
    rect_.translate(0.0, 3.0);
}

bool Player::carryBy(
    const QPointF &delta,
    const QVector<QRectF> &blockers)
{
    if (qFuzzyIsNull(delta.x()) && qFuzzyIsNull(delta.y())) {
        return true;
    }

    QRectF candidate = rect_.translated(delta.x(), 0.0);
    for (const QRectF &blocker : blockers) {
        if (candidate.intersects(blocker)) {
            return false;
        }
    }
    rect_ = candidate;

    candidate = rect_.translated(0.0, delta.y());
    for (const QRectF &blocker : blockers) {
        if (candidate.intersects(blocker)) {
            return false;
        }
    }
    rect_ = candidate;

    if (delta.y() < 0.0 && vel_.y() > 0.0f) {
        vel_.setY(0.0f);
    }

    return true;
}

bool Player::canShoot() const
{
    return shootCd_ <= 0 && ammo_ > 0;
}

Projectile Player::shoot()
{
    Projectile projectile;
    projectile.rect = QRectF(
        direction_ > 0 ? rect_.right() : rect_.left() - 16,
        rect_.top() + 34,
        16,
        7);
    projectile.velocity = QVector2D(direction_ * 680, 0);
    --ammo_;
    shootCd_ = .22;
    muzzleFlash_ = 0.10;
    return projectile;
}

void Player::damage(double sourceX)
{
    if (invuln_ > 0 || health_ <= 0) {
        return;
    }

    climbing_ = false;
    --health_;
    invuln_ = 1.0;
    vel_.setY(-400);
    vel_.setX(rect_.center().x() < sourceX ? -330 : 330);
}

void Player::heal(int amount) { health_ = std::min(5, health_ + amount); }
void Player::addAmmo(int amount) { ammo_ = std::min(99, ammo_ + amount); }
void Player::addScore(int amount) { score_ += amount; }
void Player::addKey(const QString &key) { keys_.insert(key); }
bool Player::hasKey(const QString &key) const { return keys_.contains(key); }
QRectF Player::rect() const { return rect_; }
QPointF Player::position() const { return rect_.topLeft(); }
int Player::health() const { return health_; }
int Player::ammo() const { return ammo_; }
int Player::score() const { return score_; }
bool Player::dead() const { return health_ <= 0; }

void Player::draw(QPainter &painter, double cameraX) const
{
    if (!sheet_ || !sheet_->valid()) {
        return;
    }

    if (invuln_ > 0 && int(invuln_ * 14) % 2 == 0) {
        return;
    }

    const bool moving =
        climbing_ || std::abs(vel_.x()) > 1.0f;
    const bool firing = muzzleFlash_ > 0.0;

    const int row = firing
        ? (direction_ > 0 ? 3 : 2)
        : (direction_ > 0 ? 1 : 0);

    const int column = firing
        ? (moving ? 3 : 2)
        : (moving ? 1 : 0);

    const QRect source = sheet_->frame(row, column);
    const QRectF target(
        rect_.center().x() - source.width() / 2.0 - cameraX,
        rect_.bottom() - source.height(),
        source.width(),
        source.height());

    painter.drawImage(target, sheet_->image(), source);

    if (firing) {
        const QPointF flashCenter(
            direction_ > 0
                ? rect_.right() + 13.0 - cameraX
                : rect_.left() - 13.0 - cameraX,
            rect_.top() + 35.0);

        painter.save();
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(255, 245, 120, 220));
        painter.drawEllipse(flashCenter, 10.0, 6.0);
        painter.setBrush(QColor(255, 150, 20, 210));
        painter.drawEllipse(flashCenter, 5.0, 3.0);
        painter.restore();
    }
}

Enemy::Enemy(
    const SpriteSheet *sheet,
    QString kind,
    QPointF position,
    double left,
    double right)
    : sheet_(sheet),
      kind_(kind),
      rect_(
          position.x(),
          position.y(),
          46,
          kind == "boss" ? 100 : 68),
      left_(left),
      right_(right)
{
    if (kind == "shooter") {
        speed_ = 70;
        hp_ = 2;
    } else if (kind == "jumper") {
        speed_ = 105;
        hp_ = 2;
    } else if (kind == "boss") {
        speed_ = 75;
        hp_ = 12;
        rect_.setWidth(70);
    }

    direction_ = int(position.x()) % 2 ? 1 : -1;
}

void Enemy::update(
    double dt,
    const QVector<QRectF> &platforms,
    const QVector<QRectF> &oneWayPlatforms,
    QPointF player,
    QVector<Projectile> &shots,
    bool inWater,
    double waterDrag,
    double buoyancy)
{
    if (hp_ <= 0) {
        return;
    }

    const double dx = player.x() - rect_.center().x();
    const double dy = player.y() - rect_.center().y();
    const double distanceX = std::abs(dx);

    shootCd_ -= dt;
    jumpCd_ = std::max(0.0, jumpCd_ - dt);
    hitStun_ = std::max(0.0, hitStun_ - dt);
    hurtFlash_ = std::max(0.0, hurtFlash_ - dt);

    bool wantsToMove = true;
    const double moveSpeed = speed_ * (inWater ? 0.42 : 1.0);

    if (hitStun_ > 0.0) {
        vel_.setX(vel_.x() * 0.90f);
    } else if (kind_ == "shooter") {
        direction_ = dx >= 0.0 ? 1 : -1;

        if (distanceX > 360.0) {
            vel_.setX(direction_ * moveSpeed);
        } else if (distanceX < 180.0) {
            vel_.setX(-direction_ * moveSpeed);
        } else {
            vel_.setX(0.0f);
            wantsToMove = false;
        }
    } else if (kind_ == "boss") {
        direction_ = dx >= 0.0 ? 1 : -1;
        vel_.setX(direction_ * moveSpeed);
    } else {
        vel_.setX(direction_ * moveSpeed);
    }

    bool standing = false;
    const QRectF feetProbe(
        rect_.left() + 5.0,
        rect_.bottom(),
        rect_.width() - 10.0,
        5.0);

    for (const QRectF &platform : platforms) {
        if (feetProbe.intersects(platform)) {
            standing = true;
            break;
        }
    }

    if (!standing) {
        for (const QRectF &platform : oneWayPlatforms) {
            if (feetProbe.intersects(platform)) {
                standing = true;
                break;
            }
        }
    }

    if (kind_ == "jumper"
        && jumpCd_ <= 0.0
        && distanceX < 300.0
        && std::abs(dy) < 130.0
        && standing) {
        direction_ = dx >= 0.0 ? 1 : -1;
        vel_.setX(direction_ * moveSpeed * 1.25);
        vel_.setY(-620.0f);
        jumpCd_ = 1.1;
    }

    if (inWater) {
        const double waterGravity = Gravity * (1.0 - buoyancy) * 0.55;
        vel_.setY(static_cast<float>(
            std::min(260.0, vel_.y() + waterGravity * dt)));
        const float dragFactor = static_cast<float>(
            std::exp(-waterDrag * 2.2 * dt));
        vel_ *= dragFactor;
    } else {
        vel_.setY(std::min(
            static_cast<double>(vel_.y()) + Gravity * dt,
            900.0));
    }

    const auto result = moveAndCollideOneWay(
        rect_,
        vel_,
        dt,
        platforms,
        oneWayPlatforms);

    if (result.hitWall) {
        direction_ *= -1;
    }

    if (result.onGround && wantsToMove && std::abs(vel_.x()) > 1.0f) {
        const double probeX =
            vel_.x() > 0.0f
            ? rect_.right() + 4.0
            : rect_.left() - 8.0;

        const QRectF groundProbe(
            probeX,
            rect_.bottom() + 2.0,
            4.0,
            12.0);

        bool supported = false;
        for (const QRectF &platform : platforms) {
            if (groundProbe.intersects(platform)) {
                supported = true;
                break;
            }
        }

        if (!supported) {
            direction_ *= -1;
            vel_.setX(0.0f);
        }
    }

    if (rect_.left() <= left_) {
        rect_.moveLeft(left_);
        direction_ = 1;
    }

    if (rect_.right() >= right_) {
        rect_.moveRight(right_);
        direction_ = -1;
    }

    const bool canAim =
        std::abs(dy) < 140.0
        && distanceX < (kind_ == "boss" ? 700.0 : 620.0);

    if ((kind_ == "shooter" || kind_ == "boss")
        && canAim
        && shootCd_ <= 0.0) {
        Projectile projectile;
        projectile.hostile = true;
        projectile.rect = QRectF(
            direction_ > 0
                ? rect_.right()
                : rect_.left() - 13.0,
            rect_.top() + 25.0,
            13.0,
            7.0);
        projectile.velocity = QVector2D(
            direction_ * (kind_ == "boss" ? 500.0f : 390.0f),
            kind_ == "boss" ? -40.0f : 0.0f);

        shots << projectile;
        shootCd_ = kind_ == "boss" ? 0.65 : 1.35;
    }

    anim_ += dt;
    if (anim_ > .18) {
        anim_ -= .18;
        frame_ ^= 1;
    }
}

void Enemy::damage(int amount, double sourceX)
{
    if (hp_ <= 0) {
        return;
    }

    hp_ -= amount;
    hurtFlash_ = 0.16;
    hitStun_ = 0.14;

    const float recoilDirection =
        rect_.center().x() < sourceX ? -1.0f : 1.0f;

    vel_.setX(recoilDirection * 210.0f);
    vel_.setY(-120.0f);
}

void Enemy::separateFrom(const QRectF &other)
{
    if (hp_ <= 0 || !rect_.intersects(other)) {
        return;
    }

    const QRectF overlap = rect_.intersected(other);
    if (overlap.width() <= 0.0 || overlap.height() <= 8.0) {
        return;
    }

    const double push =
        std::min(overlap.width() / 2.0 + 0.5, 6.0);

    if (rect_.center().x() < other.center().x()) {
        rect_.translate(-push, 0.0);
    } else {
        rect_.translate(push, 0.0);
    }

    if (rect_.left() < left_) {
        rect_.moveLeft(left_);
    }
    if (rect_.right() > right_) {
        rect_.moveRight(right_);
    }
}

void Enemy::draw(QPainter &painter, double cameraX) const
{
    if (hp_ <= 0 || !sheet_ || !sheet_->valid()) {
        return;
    }

    const int row =
        kind_ == "boss" ? 10
        : kind_ == "jumper" ? 6
        : kind_ == "shooter" ? 4
        : 2;

    const QRect source =
        sheet_->frame(row % 11, (direction_ > 0 ? 2 : 0) + frame_);

    const QRectF target(
        rect_.center().x() - source.width() / 2.0 - cameraX,
        rect_.bottom() - source.height(),
        source.width(),
        source.height());

    painter.save();
    if (hurtFlash_ > 0.0) {
        painter.setOpacity(0.45);
    }
    painter.drawImage(target, sheet_->image(), source);
    painter.restore();

    if (kind_ == "boss") {
        painter.fillRect(
            QRectF(
                rect_.left() - cameraX,
                rect_.top() - 10,
                rect_.width(),
                6),
            Qt::darkRed);
        painter.fillRect(
            QRectF(
                rect_.left() - cameraX,
                rect_.top() - 10,
                rect_.width() * std::max(0, hp_) / 12.0,
                6),
            Qt::red);
    }
}

QRectF Enemy::rect() const { return rect_; }
bool Enemy::alive() const { return hp_ > 0; }

int Enemy::reward() const
{
    return kind_ == "boss" ? 1000
        : kind_ == "walker" ? 100
        : 200;
}
