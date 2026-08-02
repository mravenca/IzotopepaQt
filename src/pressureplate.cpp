#include "pressureplate.h"

#include <algorithm>
#include <utility>

PressurePlate::PressurePlate(
    const QRectF &rect,
    QString target,
    double requiredWeight)
    : rect_(rect),
      target_(std::move(target)),
      requiredWeight_(std::max(0.1, requiredWeight))
{
}

void PressurePlate::update(
    double totalWeight,
    double dt,
    WorldEventQueue &events)
{
    const bool nextPressed = totalWeight >= requiredWeight_;

    if (nextPressed != pressed_) {
        pressed_ = nextPressed;
        events.postSignal(target_, pressed_);
    }

    const double desired = pressed_ ? 1.0 : 0.0;
    compression_ +=
        (desired - compression_)
        * std::min(1.0, dt * 14.0);
}

void PressurePlate::draw(QPainter &painter, double cameraX) const
{
    const QRectF worldRect = rect_.translated(-cameraX, 0.0);
    const double topOffset = compression_ * (worldRect.height() * 0.42);

    const QRectF base(
        worldRect.left(),
        worldRect.top() + worldRect.height() * 0.48,
        worldRect.width(),
        worldRect.height() * 0.52);

    const QRectF top(
        worldRect.left() + 3.0,
        worldRect.top() + topOffset,
        worldRect.width() - 6.0,
        std::max(4.0, worldRect.height() * 0.48 - topOffset));

    const QColor baseColor(50, 54, 64);
    const QColor plateColor = pressed_
        ? QColor(75, 200, 105)
        : QColor(225, 165, 55);
    const QColor edgeColor = pressed_
        ? QColor(190, 255, 205)
        : QColor(255, 225, 135);

    painter.save();
    painter.setPen(QPen(QColor(25, 28, 34), 2.0));
    painter.setBrush(baseColor);
    painter.drawRoundedRect(base, 3.0, 3.0);

    painter.setPen(QPen(edgeColor, 2.0));
    painter.setBrush(plateColor);
    painter.drawRoundedRect(top, 3.0, 3.0);

    const QPointF lamp(worldRect.center().x(), base.center().y());
    painter.setPen(Qt::NoPen);
    painter.setBrush(pressed_ ? QColor(90, 255, 120) : QColor(130, 45, 35));
    painter.drawEllipse(lamp, 3.0, 3.0);
    painter.restore();
}

QRectF PressurePlate::rect() const
{
    return rect_;
}

QRectF PressurePlate::triggerZone() const
{
    return QRectF(
        rect_.left() + 3.0,
        rect_.top() - 8.0,
        rect_.width() - 6.0,
        rect_.height() + 10.0);
}

QString PressurePlate::target() const
{
    return target_;
}

bool PressurePlate::pressed() const
{
    return pressed_;
}

double PressurePlate::requiredWeight() const
{
    return requiredWeight_;
}
