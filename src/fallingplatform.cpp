#include "fallingplatform.h"

#include <QtMath>
#include <algorithm>

namespace {
QString normalizedMaterial(QString material)
{
    material = material.trimmed().toLower();
    if (material != "wood"
        && material != "metal"
        && material != "ice") {
        return "stone";
    }
    return material;
}
}

FallingPlatform::FallingPlatform(
    const QRectF &rect,
    const QString &material,
    double confirmationTime,
    double fallDelay,
    double respawnDelay)
    : startRect_(rect),
      rect_(rect),
      material_(normalizedMaterial(material)),
      confirmationTime_(std::max(0.01, confirmationTime)),
      fallDelay_(fallDelay > 0.0
                     ? fallDelay
                     : defaultFallDelay(material_)),
      respawnDelay_(std::max(0.0, respawnDelay)),
      gravity_(defaultGravity(material_)),
      remainingTime_(fallDelay_)
{
}

FallingPlatformUpdate FallingPlatform::update(
    double dt,
    bool occupied,
    bool respawnAreaClear,
    double worldHeight)
{
    FallingPlatformUpdate result;
    animationTime_ += dt;

    switch (state_) {
    case FallingPlatformState::Idle:
        if (occupied) {
            contactTime_ += dt;
            if (contactTime_ >= confirmationTime_) {
                state_ = FallingPlatformState::Warning;
                remainingTime_ = fallDelay_;
                contactTime_ = 0.0;
                result.armed = true;
            }
        } else {
            contactTime_ = 0.0;
        }
        break;

    case FallingPlatformState::Warning:
        remainingTime_ = std::max(0.0, remainingTime_ - dt);
        if (remainingTime_ <= 0.0) {
            state_ = FallingPlatformState::Falling;
            verticalVelocity_ = 0.0;
            result.beganFalling = true;
        }
        break;

    case FallingPlatformState::Falling:
        verticalVelocity_ = std::min(
            verticalVelocity_ + gravity_ * dt,
            1500.0);
        rect_.translate(0.0, verticalVelocity_ * dt);

        if (rect_.top() > worldHeight + 120.0) {
            state_ = FallingPlatformState::WaitingToRespawn;
            respawnTimer_ = respawnDelay_;
        }
        break;

    case FallingPlatformState::WaitingToRespawn:
        respawnTimer_ = std::max(0.0, respawnTimer_ - dt);
        if (respawnTimer_ <= 0.0 && respawnAreaClear) {
            rect_ = startRect_;
            state_ = FallingPlatformState::Idle;
            contactTime_ = 0.0;
            remainingTime_ = fallDelay_;
            verticalVelocity_ = 0.0;
            result.respawned = true;
        }
        break;
    }

    return result;
}

void FallingPlatform::draw(QPainter &painter, double cameraX) const
{
    if (state_ == FallingPlatformState::WaitingToRespawn) {
        return;
    }

    QRectF target = rect_.translated(-cameraX, 0.0);
    const bool warningVisible =
        state_ == FallingPlatformState::Warning
        && remainingTime_ <= warningWindow();

    if (warningVisible) {
        const double intensity =
            1.0 - remainingTime_ / warningWindow();
        const double shake =
            qSin(animationTime_ * (12.0 + intensity * 28.0))
            * (0.6 + intensity * 3.0);
        target.translate(shake, 0.0);
    }

    painter.save();

    if (material_ == "wood") {
        painter.setBrush(QColor(143, 87, 43));
        painter.setPen(QPen(QColor(73, 40, 20), 3));
        painter.drawRect(target);
        painter.setPen(QPen(QColor(205, 139, 72), 2));
        for (double x = target.left() + 14.0;
             x < target.right(); x += 28.0) {
            painter.drawLine(
                QPointF(x, target.top() + 3.0),
                QPointF(x, target.bottom() - 3.0));
        }
    } else if (material_ == "metal") {
        painter.setBrush(QColor(90, 105, 116));
        painter.setPen(QPen(QColor(38, 49, 58), 3));
        painter.drawRoundedRect(target, 3, 3);
        painter.fillRect(
            QRectF(target.left() + 3.0, target.top() + 3.0,
                   target.width() - 6.0, 5.0),
            QColor(165, 180, 188));
        const QColor lamp = warningVisible
            && int(animationTime_ * 10.0) % 2 == 0
            ? QColor(255, 70, 45)
            : QColor(85, 35, 30);
        painter.setBrush(lamp);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(
            QPointF(target.left() + 10.0, target.center().y()),
            3.5, 3.5);
        painter.drawEllipse(
            QPointF(target.right() - 10.0, target.center().y()),
            3.5, 3.5);
    } else if (material_ == "ice") {
        painter.setBrush(QColor(112, 207, 235, 210));
        painter.setPen(QPen(QColor(215, 250, 255), 3));
        painter.drawRoundedRect(target, 4, 4);
        painter.setPen(QPen(QColor(55, 145, 190), 2));
        painter.drawLine(
            target.topLeft() + QPointF(8, 5),
            target.bottomRight() - QPointF(15, 4));
    } else {
        painter.setBrush(QColor(115, 113, 108));
        painter.setPen(QPen(QColor(55, 54, 52), 3));
        painter.drawRect(target);
        painter.fillRect(
            QRectF(target.left() + 2.0, target.top() + 2.0,
                   target.width() - 4.0, 5.0),
            QColor(170, 165, 153));
    }

    if (warningVisible) {
        const double intensity =
            1.0 - remainingTime_ / warningWindow();
        painter.setPen(QPen(
            material_ == "ice"
                ? QColor(25, 105, 155)
                : QColor(55, 35, 25),
            1.5 + intensity * 1.5));

        const double cx = target.center().x();
        painter.drawLine(
            QPointF(cx - 22.0, target.top() + 3.0),
            QPointF(cx - 8.0, target.center().y()));
        painter.drawLine(
            QPointF(cx - 8.0, target.center().y()),
            QPointF(cx - 15.0, target.bottom() - 3.0));
        painter.drawLine(
            QPointF(cx + 18.0, target.top() + 4.0),
            QPointF(cx + 5.0, target.bottom() - 3.0));
    }

    painter.restore();
}

const QRectF &FallingPlatform::rect() const { return rect_; }
const QRectF &FallingPlatform::startRect() const { return startRect_; }
FallingPlatformState FallingPlatform::state() const { return state_; }
bool FallingPlatform::isSolid() const
{
    return state_ == FallingPlatformState::Idle
        || state_ == FallingPlatformState::Warning;
}
QString FallingPlatform::material() const { return material_; }
double FallingPlatform::remainingTime() const { return remainingTime_; }

double FallingPlatform::defaultFallDelay(const QString &material)
{
    if (material == "wood") return 5.0;
    if (material == "metal") return 10.0;
    if (material == "ice") return 3.0;
    return 8.0;
}

double FallingPlatform::defaultGravity(const QString &material)
{
    if (material == "wood") return 1850.0;
    if (material == "metal") return 1100.0;
    if (material == "ice") return 1650.0;
    return 1500.0;
}

double FallingPlatform::warningWindow() const
{
    return std::min(3.0, fallDelay_);
}
