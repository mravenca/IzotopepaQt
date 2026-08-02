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
    direction_ = 1;
    shootCd_ = invuln_ = anim_ = 0;

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
    const QVector<QRectF> &ladders,
    double worldWidth)
{
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
    } else {
        vel_.setX(horizontal * WalkSpeed);

        if (horizontal != 0) {
            direction_ = horizontal > 0 ? 1 : -1;
        }

        vel_.setY(std::min(
            static_cast<double>(vel_.y()) + Gravity * dt,
            1000.0));
    }

    onGround_ =
        moveAndCollide(rect_, vel_, dt, collisionPlatforms).onGround;

    if (rect_.left() < 0.0) {
        rect_.moveLeft(0.0);
    }

    if (rect_.right() > worldWidth) {
        rect_.moveRight(worldWidth);
    }

    shootCd_ = std::max(0.0, shootCd_ - dt);
    invuln_ = std::max(0.0, invuln_ - dt);
    anim_ += dt;
}

void Player::setLeft(bool value) { left_ = value; }
void Player::setRight(bool value) { right_ = value; }
void Player::setUp(bool value) { up_ = value; }
void Player::setDown(bool value) { down_ = value; }

void Player::jump()
{
    if (onGround_ || onLadder_ || climbing_) {
        climbing_ = false;
        vel_.setY(-720.0f);
        onGround_ = false;
    }
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

    const int row = direction_ > 0 ? 1 : 0;
    const int column =
        (climbing_ || std::abs(vel_.x()) > 1.0f) ? 1 : 0;
    const QRect source = sheet_->frame(row, column);
    const QRectF target(
        rect_.center().x() - source.width() / 2.0 - cameraX,
        rect_.bottom() - source.height(),
        source.width(),
        source.height());

    painter.drawImage(target, sheet_->image(), source);
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
        speed_ = 55;
        hp_ = 2;
    } else if (kind == "jumper") {
        speed_ = 115;
        hp_ = 2;
    } else if (kind == "boss") {
        speed_ = 70;
        hp_ = 12;
        rect_.setWidth(70);
    }

    direction_ = int(position.x()) % 2 ? 1 : -1;
}

void Enemy::update(
    double dt,
    const QVector<QRectF> &platforms,
    QPointF player,
    QVector<Projectile> &shots)
{
    if (hp_ <= 0) {
        return;
    }

    const double dx = player.x() - rect_.center().x();

    if (kind_ == "boss" || kind_ == "shooter") {
        direction_ = dx > 0 ? 1 : -1;
    }

    vel_.setX(direction_ * speed_);

    if (kind_ == "jumper"
        && std::abs(dx) < 280
        && std::abs(vel_.y()) < 1) {
        vel_.setY(-620);
    }

    vel_.setY(std::min(
        static_cast<double>(vel_.y()) + Gravity * dt,
        900.0));

    const auto result =
        moveAndCollide(rect_, vel_, dt, platforms);

    if (result.hitWall) {
        direction_ *= -1;
    }

    if (rect_.left() <= left_) {
        rect_.moveLeft(left_);
        direction_ = 1;
    }

    if (rect_.right() >= right_) {
        rect_.moveRight(right_);
        direction_ = -1;
    }

    shootCd_ -= dt;

    if ((kind_ == "shooter" || kind_ == "boss")
        && std::abs(dx) < 620
        && shootCd_ <= 0) {
        Projectile projectile;
        projectile.hostile = true;
        projectile.rect =
            QRectF(rect_.center().x(), rect_.top() + 25, 13, 7);
        projectile.velocity = QVector2D(
            (dx > 0 ? 1 : -1) * (kind_ == "boss" ? 500 : 380),
            kind_ == "boss" ? -40 : 0);
        shots << projectile;
        shootCd_ = kind_ == "boss" ? .65 : 1.35;
    }

    anim_ += dt;

    if (anim_ > .18) {
        anim_ -= .18;
        frame_ ^= 1;
    }
}

void Enemy::damage(int amount) { hp_ -= amount; }

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

    painter.drawImage(target, sheet_->image(), source);

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
