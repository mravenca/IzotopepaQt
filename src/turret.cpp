#include "turret.h"

#include <QtMath>
#include <algorithm>

namespace {
double normalizeAngle(double angle)
{
    while (angle > 180.0) angle -= 360.0;
    while (angle < -180.0) angle += 360.0;
    return angle;
}

double moveAngleTowards(double current, double target, double maximumDelta)
{
    const double difference = normalizeAngle(target - current);
    if (qAbs(difference) <= maximumDelta) {
        return target;
    }
    return normalizeAngle(current + (difference > 0.0 ? maximumDelta : -maximumDelta));
}

QVector2D directionFromAngle(double degrees)
{
    const double radians = qDegreesToRadians(degrees);
    return QVector2D(
        static_cast<float>(qCos(radians)),
        static_cast<float>(qSin(radians)));
}
}

Turret::Turret(const TurretConfig &config)
    : rect_(config.position.x(), config.position.y(), 50, 46),
      brain_(config.vision, config.reload),
      mount_(config.mount.toLower()),
      baseAngle_(config.direction >= 0 ? 0.0 : 180.0),
      aimAngle_(baseAngle_),
      visionAngle_(std::clamp(config.visionAngle, 20.0, 300.0)),
      rotationSpeed_(std::max(20.0, config.rotationSpeed)),
      projectileSpeed_(std::max(80.0, config.projectileSpeed)),
      health_(std::max(1, config.health)),
      burstSize_(std::clamp(config.burst, 1, 8))
{
    if (mount_ != "ceiling") {
        mount_ = "floor";
    }
}

bool Turret::playerInVisionCone(const QPointF &player) const
{
    const QPointF delta = player - rect_.center();
    const double distance = qSqrt(
        delta.x() * delta.x() + delta.y() * delta.y());

    if (distance > brain_.visionRange()) {
        return false;
    }

    const double targetAngle = qRadiansToDegrees(qAtan2(delta.y(), delta.x()));
    return qAbs(normalizeAngle(targetAngle - baseAngle_)) <= visionAngle_ * 0.5;
}

void Turret::update(
    double dt,
    const QPointF &player,
    QVector<Projectile> &shots)
{
    if (!alive()) {
        return;
    }

    hurtFlash_ = std::max(0.0, hurtFlash_ - dt);
    recoil_ = std::max(0.0, recoil_ - dt * 6.0);
    muzzleFlash_ = std::max(0.0, muzzleFlash_ - dt);
    burstTimer_ = std::max(0.0, burstTimer_ - dt);

    const bool visible = playerInVisionCone(player);
    brain_.updateVisible(dt, rect_.center(), player, visible);

    if (visible) {
        const QPointF delta = player - rect_.center();
        const double targetAngle = qRadiansToDegrees(qAtan2(delta.y(), delta.x()));
        aimAngle_ = moveAngleTowards(
            aimAngle_,
            targetAngle,
            rotationSpeed_ * dt);
    } else {
        aimAngle_ = moveAngleTowards(
            aimAngle_,
            baseAngle_,
            rotationSpeed_ * 0.45 * dt);
    }

    if (burstRemaining_ <= 0 && brain_.canFire()) {
        burstRemaining_ = burstSize_;
        burstTimer_ = 0.0;
        brain_.consumeShot();
    }

    if (burstRemaining_ > 0 && burstTimer_ <= 0.0 && visible) {
        const QPointF delta = player - rect_.center();
        const double targetAngle = qRadiansToDegrees(qAtan2(delta.y(), delta.x()));

        if (qAbs(normalizeAngle(targetAngle - aimAngle_)) <= 9.0) {
            fireOneShot(player, shots);
            --burstRemaining_;
            burstTimer_ = 0.13;
        }
    }
}

void Turret::fireOneShot(
    const QPointF &player,
    QVector<Projectile> &shots)
{
    Q_UNUSED(player);

    const QVector2D direction = directionFromAngle(aimAngle_);
    const QPointF muzzle = rect_.center() + direction.toPointF() * 28.0;

    Projectile projectile;
    projectile.hostile = true;
    projectile.rect = QRectF(muzzle.x() - 5, muzzle.y() - 3, 10, 6);
    projectile.velocity = direction * static_cast<float>(projectileSpeed_);
    projectile.life = 3.0;
    shots << projectile;

    recoil_ = 1.0;
    muzzleFlash_ = 0.09;
}

void Turret::damage(int amount, const QPointF &source)
{
    Q_UNUSED(source);
    if (!alive()) {
        return;
    }

    health_ -= std::max(1, amount);
    hurtFlash_ = 0.18;
}

void Turret::applyExplosion(const QPointF &center, int damage)
{
    Q_UNUSED(center);
    if (!alive()) {
        return;
    }

    health_ -= std::max(2, damage * 2);
    hurtFlash_ = 0.25;
}

void Turret::draw(QPainter &painter, double cameraX, double time) const
{
    if (!alive()) {
        return;
    }

    const QRectF body = rect_.translated(-cameraX, 0);
    const QPointF pivot = body.center();
    const QVector2D direction = directionFromAngle(aimAngle_);
    const QPointF recoilOffset = -direction.toPointF() * recoil_ * 5.0;

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(QPen(QColor(28, 30, 34), 3));
    painter.setBrush(
        hurtFlash_ > 0.0
            ? QColor(255, 240, 240)
            : QColor(92, 102, 112));
    painter.drawRoundedRect(body, 7, 7);

    painter.setBrush(QColor(45, 50, 56));
    if (mount_ == "ceiling") {
        painter.drawRect(QRectF(body.left() + 7, body.top() - 7, body.width() - 14, 9));
    } else {
        painter.drawRect(QRectF(body.left() + 7, body.bottom() - 2, body.width() - 14, 9));
    }

    painter.setPen(QPen(QColor(38, 42, 48), 10, Qt::SolidLine, Qt::RoundCap));
    const QPointF barrelStart = pivot + recoilOffset;
    const QPointF barrelEnd = barrelStart + direction.toPointF() * 31.0;
    painter.drawLine(barrelStart, barrelEnd);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(225, 55, 45));
    painter.setOpacity(qSin(time * 4.0) > -0.75 ? 1.0 : 0.35);
    painter.drawEllipse(pivot + QPointF(0, -9), 4, 4);
    painter.setOpacity(1.0);

    if (muzzleFlash_ > 0.0) {
        painter.setBrush(QColor(255, 235, 105, 225));
        painter.drawEllipse(barrelEnd, 9, 6);
        painter.setBrush(QColor(255, 125, 25, 220));
        painter.drawEllipse(barrelEnd, 4, 3);
    }

    painter.restore();
}

QRectF Turret::rect() const { return rect_; }
bool Turret::alive() const { return health_ > 0; }
int Turret::reward() const { return 300; }

QString Turret::debugText() const
{
    return QString(
        "Turret  State: %1\nHP: %2  Vision: %3 / %4 deg\n"
        "Cooldown: %5  Burst remaining: %6  Mount: %7")
        .arg(brain_.stateName())
        .arg(std::max(0, health_))
        .arg(brain_.visionRange(), 0, 'f', 0)
        .arg(visionAngle_, 0, 'f', 0)
        .arg(brain_.fireCooldown(), 0, 'f', 2)
        .arg(burstRemaining_)
        .arg(mount_);
}
