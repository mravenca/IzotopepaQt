#include "drone.h"

#include <QLineF>
#include <QtMath>
#include <algorithm>

namespace {
QVector2D normalizedOrZero(const QPointF &delta)
{
    QVector2D result(delta);
    if (result.lengthSquared() > 0.0001f) {
        result.normalize();
    }
    return result;
}
}

Drone::Drone(const DroneConfig &config)
    : rect_(config.position.x(), config.position.y(), 58, 34),
      patrol_(config.patrol),
      brain_(config.vision, config.reload),
      speed_(std::max(20.0, config.speed)),
      health_(std::max(1, config.health)),
      burstSize_(std::clamp(config.burst, 1, 6))
{
    if (patrol_.isEmpty()) {
        patrol_ << config.position + QPointF(-140, 0)
                << config.position + QPointF(140, 0);
    }
}

QPointF Drone::patrolTarget() const
{
    if (patrol_.isEmpty()) {
        return rect_.center();
    }

    const int lastIndex =
        static_cast<int>(patrol_.size()) - 1;

    return patrol_[std::clamp(
        waypoint_,
        0,
        lastIndex)];
}

void Drone::advanceWaypoint()
{
    if (!patrol_.isEmpty()) {
        waypoint_ = (waypoint_ + 1) % patrol_.size();
    }
}

void Drone::update(
    double dt,
    const QPointF &player,
    const QVector<QRectF> &solidGeometry,
    QVector<Projectile> &shots)
{
    if (!alive()) {
        return;
    }

    hoverPhase_ += dt * 3.2;
    hurtFlash_ = std::max(0.0, hurtFlash_ - dt);
    recoil_ = std::max(0.0, recoil_ - dt * 4.5);

    brain_.update(dt, rect_.center(), player);

    QPointF target = patrolTarget();
    double desiredSpeed = speed_;

    if (brain_.seesTarget()) {
        const QPointF offset(
            player.x() < rect_.center().x() ? 165.0 : -165.0,
            -90.0);
        target = player + offset;
        desiredSpeed *= 1.12;
    } else if (brain_.state() == EnemyState::Search) {
        target = brain_.lastKnownTarget() + QPointF(0, -80);
    }

    const QPointF delta = target - rect_.center();
    if (!brain_.seesTarget() && QLineF(rect_.center(), target).length() < 18.0) {
        advanceWaypoint();
        target = patrolTarget();
    }

    QVector2D desired = normalizedOrZero(target - rect_.center());
    desired *= static_cast<float>(desiredSpeed);

    const float steering = static_cast<float>(std::min(1.0, dt * 4.2));
    velocity_ += (desired - velocity_) * steering;
    velocity_.setY(velocity_.y() + static_cast<float>(qSin(hoverPhase_) * 5.0));

    if (qAbs(velocity_.x()) > 3.0f) {
        direction_ = velocity_.x() > 0.0f ? 1 : -1;
    }

    QRectF proposed = rect_.translated(
        velocity_.x() * dt,
        velocity_.y() * dt);

    bool blocked = false;
    for (const QRectF &solid : solidGeometry) {
        if (proposed.intersects(solid)) {
            blocked = true;
            break;
        }
    }

    if (blocked) {
        velocity_ *= -0.35f;
        rect_.translate(velocity_.x() * dt, velocity_.y() * dt);
        if (!brain_.seesTarget()) {
            advanceWaypoint();
        }
    } else {
        rect_ = proposed;
    }

    if (brain_.canFire()
        && qAbs(player.y() - rect_.center().y()) < 260.0) {
        fireBurst(player, shots);
        brain_.consumeShot();
        recoil_ = 1.0;
    }
}

void Drone::fireBurst(
    const QPointF &player,
    QVector<Projectile> &shots)
{
    const QVector2D direction = normalizedOrZero(player - rect_.center());

    for (int index = 0; index < burstSize_; ++index) {
        const double spread = (index - (burstSize_ - 1) * 0.5) * 0.055;
        const double c = qCos(spread);
        const double s = qSin(spread);
        const QVector2D shotDirection(
            direction.x() * c - direction.y() * s,
            direction.x() * s + direction.y() * c);

        Projectile projectile;
        projectile.hostile = true;
        projectile.rect = QRectF(
            rect_.center().x() - 5,
            rect_.center().y() - 3,
            10,
            6);
        projectile.velocity = shotDirection * 360.0f;
        projectile.life = 2.8;
        shots << projectile;
    }
}

void Drone::damage(int amount, const QPointF &source)
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
    velocity_ += knockback * 190.0f;
}

void Drone::applyExplosion(const QPointF &center, int damage)
{
    damage = std::max(1, damage);
    health_ -= damage;

    QVector2D impulse(rect_.center() - center);
    if (impulse.lengthSquared() > 0.001f) {
        impulse.normalize();
    }
    velocity_ += impulse * 360.0f;
    hurtFlash_ = 0.25;
}

void Drone::draw(QPainter &painter, double cameraX, double time) const
{
    if (!alive()) {
        return;
    }

    const QRectF body = rect_.translated(-cameraX, qSin(time * 4.0 + hoverPhase_) * 2.5);
    const QColor bodyColor = hurtFlash_ > 0.0
        ? QColor(255, 245, 245)
        : QColor(72, 83, 96);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(QPen(QColor(25, 30, 36), 3));
    painter.setBrush(bodyColor);
    painter.drawRoundedRect(body, 10, 10);

    const double rotorWidth = 22.0 + qSin(time * 28.0) * 5.0;
    painter.setPen(QPen(QColor(205, 220, 230), 3));
    painter.drawLine(
        QPointF(body.left() - rotorWidth, body.top() + 6),
        QPointF(body.left() + 8, body.top() + 6));
    painter.drawLine(
        QPointF(body.right() - 8, body.top() + 6),
        QPointF(body.right() + rotorWidth, body.top() + 6));

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(225, 55, 45));
    const double blink = qSin(time * 5.0) > -0.65 ? 1.0 : 0.35;
    painter.setOpacity(blink);
    painter.drawEllipse(body.center() + QPointF(direction_ * 9.0, 0), 5, 5);
    painter.setOpacity(1.0);

    painter.setBrush(QColor(90, 190, 255, 150));
    painter.drawEllipse(
        body.center() + QPointF(-direction_ * 22.0, 12.0),
        4.0 + recoil_ * 3.0,
        2.5);

    painter.restore();
}

QRectF Drone::rect() const { return rect_; }
bool Drone::alive() const { return health_ > 0; }
int Drone::reward() const { return 250; }

QString Drone::debugText() const
{
    return QString(
        "Drone  State: %1\nHP: %2  Vision: %3\nCooldown: %4  Waypoint: %5/%6")
        .arg(brain_.stateName())
        .arg(std::max(0, health_))
        .arg(brain_.visionRange(), 0, 'f', 0)
        .arg(brain_.fireCooldown(), 0, 'f', 2)
        .arg(waypoint_ + 1)
        .arg(patrol_.size());
}
