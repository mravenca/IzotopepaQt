#include "shieldsoldier.h"

#include "support.h"

#include <QtMath>
#include <algorithm>
#include <cmath>

namespace {
constexpr double Gravity = 1900.0;
constexpr double ProjectileSpeed = 390.0;
}

ShieldSoldier::ShieldSoldier(const ShieldSoldierConfig &config)
    : rect_(config.position.x(), config.position.y(), 52, 72),
      leftLimit_(config.leftLimit),
      rightLimit_(config.rightLimit),
      vision_(std::max(80.0, config.vision)),
      moveSpeed_(std::max(35.0, config.speed)),
      shieldAngle_(std::clamp(config.shieldAngle, 45.0, 180.0)),
      fireCooldownTime_(std::max(0.3, config.fireCooldown)),
      health_(std::max(1, config.health))
{
    if (rightLimit_ <= leftLimit_) {
        leftLimit_ = config.position.x() - 150.0;
        rightLimit_ = config.position.x() + 210.0;
    }
    direction_ = int(config.position.x()) % 2 == 0 ? 1 : -1;
}

void ShieldSoldier::enterState(ShieldState state, double duration)
{
    state_ = state;
    stateTimer_ = duration;
}

bool ShieldSoldier::sourceInShieldArc(const QPointF &source) const
{
    const QPointF delta = source - rect_.center();
    if (delta.y() < -rect_.height() * 0.32) {
        return false;
    }

    const double angle = qRadiansToDegrees(qAtan2(delta.y(), delta.x()));
    const double facingAngle = direction_ > 0 ? 0.0 : 180.0;
    double difference = angle - facingAngle;
    while (difference > 180.0) difference -= 360.0;
    while (difference < -180.0) difference += 360.0;
    return qAbs(difference) <= shieldAngle_ * 0.5;
}

void ShieldSoldier::update(
    double dt,
    const QPointF &player,
    const QVector<QRectF> &solidGeometry,
    const QVector<QRectF> &oneWayPlatforms,
    bool inWater,
    double conveyorSpeed,
    double iceFriction,
    QVector<Projectile> &shots)
{
    if (!alive()) {
        return;
    }

    animationTime_ += dt;
    hurtFlash_ = std::max(0.0, hurtFlash_ - dt);
    shieldFlash_ = std::max(0.0, shieldFlash_ - dt);
    muzzleFlash_ = std::max(0.0, muzzleFlash_ - dt);
    fireCooldown_ = std::max(0.0, fireCooldown_ - dt);
    stateTimer_ = std::max(0.0, stateTimer_ - dt);

    const double dx = player.x() - rect_.center().x();
    const double dy = player.y() - rect_.center().y();
    const bool seesPlayer = std::abs(dx) <= vision_ && std::abs(dy) <= 150.0;

    if (state_ != ShieldState::Staggered && seesPlayer) {
        direction_ = dx >= 0.0 ? 1 : -1;
    }

    switch (state_) {
    case ShieldState::Patrol:
        velocity_.setX(static_cast<float>(direction_ * moveSpeed_ * 0.62));
        if (seesPlayer) {
            enterState(ShieldState::Advance, 0.7);
        }
        break;

    case ShieldState::Advance:
        velocity_.setX(static_cast<float>(direction_ * moveSpeed_));
        if (!seesPlayer) {
            enterState(ShieldState::Patrol);
        } else if (std::abs(dx) < 250.0 || stateTimer_ <= 0.0) {
            velocity_.setX(0.0f);
            enterState(ShieldState::Defend, 0.55);
        }
        break;

    case ShieldState::Defend:
        velocity_.setX(static_cast<float>(conveyorSpeed));
        if (stateTimer_ <= 0.0) {
            enterState(ShieldState::Attack, 0.22);
        }
        break;

    case ShieldState::Attack:
        velocity_.setX(0.0f);
        if (stateTimer_ <= 0.0) {
            if (fireCooldown_ <= 0.0 && seesPlayer) {
                fire(player, shots);
                fireCooldown_ = fireCooldownTime_ * (inWater ? 1.45 : 1.0);
            }
            enterState(ShieldState::Advance, 0.85);
        }
        break;

    case ShieldState::Staggered: {
        const double friction = std::clamp(iceFriction, 0.02, 1.0);
        velocity_.setX(velocity_.x() * static_cast<float>(
            std::exp(-friction * 6.0 * dt)));
        if (stateTimer_ <= 0.0) {
            enterState(ShieldState::Patrol);
        }
        break;
    }
    }

    if (inWater) {
        velocity_.setX(velocity_.x() * static_cast<float>(std::exp(-1.9 * dt)));
        velocity_.setY(std::min(
            velocity_.y() + static_cast<float>(420.0 * dt), 280.0f));
    } else {
        velocity_.setY(std::min(
            velocity_.y() + static_cast<float>(Gravity * dt), 950.0f));
    }

    const MoveResult result = moveAndCollideOneWay(
        rect_, velocity_, dt, solidGeometry, oneWayPlatforms);

    if (result.hitWall) {
        direction_ *= -1;
        if (state_ != ShieldState::Staggered) {
            enterState(ShieldState::Patrol);
        }
    }

    if (state_ == ShieldState::Patrol) {
        if (rect_.left() <= leftLimit_) {
            rect_.moveLeft(leftLimit_);
            direction_ = 1;
        } else if (rect_.right() >= rightLimit_) {
            rect_.moveRight(rightLimit_);
            direction_ = -1;
        }
    }
}

void ShieldSoldier::fire(
    const QPointF &player,
    QVector<Projectile> &shots)
{
    QVector2D direction(player - rect_.center());
    if (direction.lengthSquared() < 0.001f) {
        direction = QVector2D(static_cast<float>(direction_), 0.0f);
    } else {
        direction.normalize();
    }

    const QPointF muzzle = rect_.center()
        + QPointF(direction_ * 29.0, -8.0);

    Projectile projectile;
    projectile.hostile = true;
    projectile.rect = QRectF(muzzle.x() - 5, muzzle.y() - 3, 10, 6);
    projectile.velocity = direction * static_cast<float>(ProjectileSpeed);
    projectile.life = 3.0;
    shots << projectile;
    muzzleFlash_ = 0.10;
}

ShieldBulletResult ShieldSoldier::receiveBullet(
    int amount,
    const QPointF &source)
{
    if (!alive()) {
        return ShieldBulletResult::Missed;
    }

    if (sourceInShieldArc(source)
        && state_ != ShieldState::Staggered) {
        shieldFlash_ = 0.16;
        velocity_.setX(static_cast<float>(-direction_ * 28.0));
        return ShieldBulletResult::Blocked;
    }

    health_ -= std::max(1, amount);
    hurtFlash_ = 0.18;
    velocity_.setX(static_cast<float>(
        source.x() < rect_.center().x() ? 105.0 : -105.0));

    if (alive()) {
        enterState(ShieldState::Staggered, 0.55);
        return ShieldBulletResult::Damaged;
    }
    return ShieldBulletResult::Destroyed;
}

void ShieldSoldier::applyExplosion(
    const QPointF &center,
    int damage)
{
    if (!alive()) {
        return;
    }

    health_ -= std::max(1, damage);
    hurtFlash_ = 0.25;

    QVector2D impulse(rect_.center() - center);
    if (impulse.lengthSquared() > 0.001f) {
        impulse.normalize();
    }
    velocity_ += impulse * 280.0f;

    if (alive()) {
        enterState(ShieldState::Staggered, 1.0);
    }
}

void ShieldSoldier::launch(const QVector2D &impulse)
{
    if (!alive()) {
        return;
    }
    velocity_ = impulse;
    enterState(ShieldState::Staggered, 1.1);
}

void ShieldSoldier::draw(
    QPainter &painter,
    double cameraX,
    double time) const
{
    if (!alive()) {
        return;
    }

    const QRectF body = rect_.translated(-cameraX, 0.0);
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    QColor armor(75, 105, 135);
    if (hurtFlash_ > 0.0) {
        armor = QColor(255, 240, 235);
    }

    painter.setPen(QPen(QColor(25, 35, 45), 3));
    painter.setBrush(armor);
    painter.drawRoundedRect(body.adjusted(8, 7, -8, -2), 7, 7);

    painter.setBrush(QColor(42, 48, 58));
    painter.drawEllipse(body.center() + QPointF(0, -23), 13, 13);

    const bool shieldRaised = state_ != ShieldState::Staggered;
    if (shieldRaised) {
        QRectF shield(
            direction_ > 0 ? body.right() - 5 : body.left() - 14,
            body.top() + 12,
            19,
            47);
        painter.setPen(QPen(
            shieldFlash_ > 0.0
                ? QColor(210, 250, 255)
                : QColor(80, 185, 225),
            shieldFlash_ > 0.0 ? 5 : 3));
        painter.setBrush(QColor(45, 105, 145, 220));
        painter.drawRoundedRect(shield, 8, 8);
        painter.setPen(QPen(QColor(145, 225, 250, 180), 2));
        painter.drawArc(shield.adjusted(4, 4, -4, -4), 60 * 16, 240 * 16);
    }

    painter.setPen(QPen(QColor(35, 38, 45), 7, Qt::SolidLine, Qt::RoundCap));
    const QPointF gunStart = body.center() + QPointF(direction_ * 4, -7);
    const QPointF gunEnd = gunStart + QPointF(direction_ * 24, 0);
    painter.drawLine(gunStart, gunEnd);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(235, 55, 45));
    painter.drawEllipse(
        body.center() + QPointF(direction_ * 7, -25), 3.5, 3.5);

    if (muzzleFlash_ > 0.0) {
        painter.setBrush(QColor(255, 235, 105, 230));
        painter.drawEllipse(gunEnd, 8, 5);
    }

    if (state_ == ShieldState::Staggered) {
        painter.setPen(QPen(QColor(255, 220, 70), 3));
        painter.drawArc(
            QRectF(body.center().x() - 18, body.top() - 12, 36, 20),
            static_cast<int>(time * 180.0) % (360 * 16),
            150 * 16);
    }

    painter.restore();
}

QRectF ShieldSoldier::rect() const { return rect_; }
QVector2D ShieldSoldier::velocity() const { return velocity_; }
bool ShieldSoldier::alive() const { return health_ > 0; }
int ShieldSoldier::reward() const { return 400; }

QString ShieldSoldier::stateName() const
{
    switch (state_) {
    case ShieldState::Patrol: return "Patrol";
    case ShieldState::Advance: return "Advance";
    case ShieldState::Defend: return "Defend";
    case ShieldState::Attack: return "Attack";
    case ShieldState::Staggered: return "Staggered";
    }
    return "Unknown";
}

QString ShieldSoldier::debugText() const
{
    return QString(
        "Shield Soldier  State: %1\nHP: %2  Shield: %3\n"
        "Cooldown: %4  Facing: %5  Arc: %6 deg")
        .arg(stateName())
        .arg(std::max(0, health_))
        .arg(state_ == ShieldState::Staggered ? "lowered" : "active")
        .arg(fireCooldown_, 0, 'f', 2)
        .arg(direction_ > 0 ? "right" : "left")
        .arg(shieldAngle_, 0, 'f', 0);
}
