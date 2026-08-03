#include "charger.h"

#include "support.h"

#include <QtMath>
#include <algorithm>
#include <cmath>

namespace {
constexpr double Gravity = 1900.0;
constexpr double PatrolSpeed = 82.0;
}

Charger::Charger(const ChargerConfig &config)
    : rect_(config.position.x(), config.position.y(), 58, 70),
      leftLimit_(config.leftLimit),
      rightLimit_(config.rightLimit),
      vision_(std::max(80.0, config.vision)),
      warningTime_(std::max(0.1, config.warningTime)),
      chargeSpeed_(std::max(180.0, config.chargeSpeed)),
      stunTime_(std::max(0.2, config.stunTime)),
      health_(std::max(1, config.health)),
      contactDamage_(std::max(1, config.contactDamage))
{
    if (rightLimit_ <= leftLimit_) {
        leftLimit_ = config.position.x() - 180.0;
        rightLimit_ = config.position.x() + 240.0;
    }

    direction_ = int(config.position.x()) % 2 == 0 ? 1 : -1;
}

void Charger::enterState(ChargerState state, double duration)
{
    state_ = state;
    stateTimer_ = duration;
}

void Charger::update(
    double dt,
    const QPointF &player,
    const QVector<QRectF> &solidGeometry,
    const QVector<QRectF> &oneWayPlatforms,
    bool inWater,
    double conveyorSpeed,
    double iceFriction)
{
    if (!alive()) {
        return;
    }

    animationTime_ += dt;
    hurtFlash_ = std::max(0.0, hurtFlash_ - dt);
    stateTimer_ = std::max(0.0, stateTimer_ - dt);

    const double dx = player.x() - rect_.center().x();
    const double dy = player.y() - rect_.center().y();
    const bool seesPlayer =
        std::abs(dx) <= vision_
        && std::abs(dy) <= 125.0
        && (dx >= 0.0 ? 1 : -1) == direction_;

    if (inWater && state_ == ChargerState::Charging) {
        enterState(ChargerState::Stunned, stunTime_ * 0.65);
    }

    switch (state_) {
    case ChargerState::Patrol:
        velocity_.setX(static_cast<float>(direction_ * PatrolSpeed));
        if (seesPlayer) {
            direction_ = dx >= 0.0 ? 1 : -1;
            velocity_.setX(0.0f);
            enterState(ChargerState::Warning, warningTime_);
        }
        break;

    case ChargerState::Warning:
        velocity_.setX(0.0f);
        direction_ = dx >= 0.0 ? 1 : -1;
        if (stateTimer_ <= 0.0) {
            enterState(ChargerState::Charging);
            velocity_.setX(static_cast<float>(direction_ * chargeSpeed_));
        }
        break;

    case ChargerState::Charging:
        velocity_.setX(static_cast<float>(direction_ * chargeSpeed_ + conveyorSpeed));
        break;

    case ChargerState::Stunned: {
        const double friction = std::clamp(iceFriction, 0.02, 1.0);
        velocity_.setX(velocity_.x() * static_cast<float>(
            std::exp(-friction * 5.0 * dt)));
        if (stateTimer_ <= 0.0) {
            enterState(ChargerState::Recovering, 0.55);
        }
        break;
    }

    case ChargerState::Recovering:
        velocity_.setX(0.0f);
        if (stateTimer_ <= 0.0) {
            direction_ *= -1;
            enterState(ChargerState::Patrol);
        }
        break;
    }

    if (inWater) {
        velocity_.setX(velocity_.x() * static_cast<float>(std::exp(-2.3 * dt)));
        velocity_.setY(std::min(velocity_.y() + static_cast<float>(350.0 * dt), 260.0f));
    } else {
        velocity_.setY(std::min(velocity_.y() + static_cast<float>(Gravity * dt), 950.0f));
    }

    const MoveResult result = moveAndCollideOneWay(
        rect_, velocity_, dt, solidGeometry, oneWayPlatforms);

    if (result.hitWall) {
        if (state_ == ChargerState::Charging) {
            enterState(ChargerState::Stunned, stunTime_);
        } else {
            direction_ *= -1;
        }
    }

    if (state_ == ChargerState::Patrol) {
        if (rect_.left() <= leftLimit_) {
            rect_.moveLeft(leftLimit_);
            direction_ = 1;
        } else if (rect_.right() >= rightLimit_) {
            rect_.moveRight(rightLimit_);
            direction_ = -1;
        }

        if (result.onGround) {
            const double probeX = direction_ > 0
                ? rect_.right() + 3.0
                : rect_.left() - 7.0;
            const QRectF groundProbe(
                probeX, rect_.bottom() + 2.0, 4.0, 14.0);
            bool supported = false;
            for (const QRectF &solid : solidGeometry) {
                if (groundProbe.intersects(solid)) {
                    supported = true;
                    break;
                }
            }
            if (!supported) {
                direction_ *= -1;
            }
        }
    }
}

void Charger::damage(int amount, const QPointF &source)
{
    if (!alive()) {
        return;
    }

    health_ -= std::max(1, amount);
    hurtFlash_ = 0.18;

    QVector2D knockback(rect_.center() - source);
    if (knockback.lengthSquared() > 0.001f) {
        knockback.normalize();
    }
    velocity_ += knockback * 180.0f;

    if (alive() && state_ == ChargerState::Charging) {
        enterState(ChargerState::Stunned, stunTime_ * 0.55);
    }
}

void Charger::applyExplosion(const QPointF &center, int damage)
{
    if (!alive()) {
        return;
    }

    health_ -= std::max(1, damage);
    hurtFlash_ = 0.28;

    QVector2D impulse(rect_.center() - center);
    if (impulse.lengthSquared() > 0.001f) {
        impulse.normalize();
    }
    velocity_ += impulse * 330.0f;

    if (alive()) {
        enterState(ChargerState::Stunned, stunTime_);
    }
}

void Charger::launch(const QVector2D &impulse)
{
    velocity_ = impulse;
    enterState(ChargerState::Stunned, stunTime_ * 0.55);
}

void Charger::stun()
{
    if (!alive()) {
        return;
    }
    velocity_.setX(-direction_ * 120.0f);
    velocity_.setY(-120.0f);
    enterState(ChargerState::Stunned, stunTime_);
}

void Charger::draw(QPainter &painter, double cameraX, double time) const
{
    if (!alive()) {
        return;
    }

    QRectF body = rect_.translated(-cameraX, 0.0);
    if (state_ == ChargerState::Warning) {
        body.translate(qSin(time * 42.0) * 2.5, 0.0);
    }

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    QColor bodyColor(165, 72, 42);
    if (state_ == ChargerState::Charging) {
        bodyColor = QColor(215, 70, 38);
    } else if (state_ == ChargerState::Stunned) {
        bodyColor = QColor(105, 110, 120);
    }
    if (hurtFlash_ > 0.0) {
        bodyColor = QColor(255, 240, 235);
    }

    painter.setPen(QPen(QColor(45, 30, 28), 3));
    painter.setBrush(bodyColor);
    painter.drawRoundedRect(body, 10, 10);

    const QPointF front = direction_ > 0
        ? body.topRight() + QPointF(5, 22)
        : body.topLeft() + QPointF(-5, 22);
    QPolygonF horn;
    if (direction_ > 0) {
        horn << front + QPointF(-3, -7)
             << front + QPointF(18, 0)
             << front + QPointF(-3, 7);
    } else {
        horn << front + QPointF(3, -7)
             << front + QPointF(-18, 0)
             << front + QPointF(3, 7);
    }
    painter.setBrush(QColor(235, 210, 150));
    painter.drawPolygon(horn);

    painter.setPen(Qt::NoPen);
    painter.setBrush(state_ == ChargerState::Warning
        ? QColor(255, 235, 80)
        : QColor(235, 45, 35));
    painter.drawEllipse(
        body.center() + QPointF(direction_ * 13.0, -12.0), 5, 5);

    painter.setBrush(QColor(45, 45, 50));
    painter.drawEllipse(body.bottomLeft() + QPointF(13, -3), 8, 8);
    painter.drawEllipse(body.bottomRight() + QPointF(-13, -3), 8, 8);

    if (state_ == ChargerState::Stunned) {
        painter.setPen(QPen(QColor(255, 225, 70), 3));
        const QPointF center = body.topLeft() + QPointF(
            body.width() * 0.5 + qSin(time * 5.0) * 18.0,
            -8.0 + qCos(time * 5.0) * 4.0);
        painter.drawEllipse(center, 3, 3);
        painter.drawEllipse(body.topLeft() + QPointF(
            body.width() * 0.5 + qSin(time * 5.0 + 3.14) * 18.0,
            -8.0 + qCos(time * 5.0 + 3.14) * 4.0), 3, 3);
    }

    painter.restore();
}

QRectF Charger::rect() const { return rect_; }
QVector2D Charger::velocity() const { return velocity_; }
bool Charger::alive() const { return health_ > 0; }
bool Charger::charging() const { return state_ == ChargerState::Charging; }
int Charger::direction() const { return direction_; }
int Charger::contactDamage() const { return contactDamage_; }
int Charger::reward() const { return 350; }

QString Charger::stateName() const
{
    switch (state_) {
    case ChargerState::Patrol: return "Patrol";
    case ChargerState::Warning: return "Warning";
    case ChargerState::Charging: return "Charging";
    case ChargerState::Stunned: return "Stunned";
    case ChargerState::Recovering: return "Recovering";
    }
    return "Unknown";
}

QString Charger::debugText() const
{
    return QString(
        "Charger  State: %1\nHP: %2  Direction: %3\n"
        "Timer: %4  Charge speed: %5")
        .arg(stateName())
        .arg(std::max(0, health_))
        .arg(direction_ > 0 ? "right" : "left")
        .arg(stateTimer_, 0, 'f', 2)
        .arg(chargeSpeed_, 0, 'f', 0);
}
