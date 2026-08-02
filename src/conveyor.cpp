#include "conveyor.h"

#include <QColor>
#include <QLinearGradient>
#include <QPen>
#include <QtMath>

#include <algorithm>
#include <cmath>

Conveyor::Conveyor(const QRectF &rect, double speed)
    : rect_(rect),
      speed_(std::clamp(speed, -500.0, 500.0))
{
}

void Conveyor::update(double dt)
{
    animationOffset_ += speed_ * dt;

    constexpr double kPattern = 24.0;
    animationOffset_ = std::fmod(animationOffset_, kPattern);
}

QRectF Conveyor::rect() const
{
    return rect_;
}

QRectF Conveyor::surfaceZone() const
{
    return QRectF(
        rect_.left() + 2.0,
        rect_.top() - 7.0,
        std::max(1.0, rect_.width() - 4.0),
        13.0);
}

double Conveyor::speed() const
{
    return speed_;
}

void Conveyor::draw(QPainter &painter, double cameraX) const
{
    const QRectF body = rect_.translated(-cameraX, 0.0);

    QLinearGradient gradient(body.topLeft(), body.bottomLeft());
    gradient.setColorAt(0.0, QColor(150, 160, 170));
    gradient.setColorAt(0.45, QColor(88, 98, 108));
    gradient.setColorAt(1.0, QColor(45, 52, 60));

    painter.save();
    painter.setPen(QPen(QColor(28, 32, 38), 3.0));
    painter.setBrush(gradient);
    painter.drawRoundedRect(body, 4.0, 4.0);

    const QRectF belt = body.adjusted(4.0, 4.0, -4.0, -5.0);
    painter.setClipRect(belt);
    painter.setPen(QPen(QColor(225, 190, 65), 3.0));

    constexpr double kPattern = 24.0;
    double offset = std::fmod(animationOffset_, kPattern);
    if (offset < 0.0) {
        offset += kPattern;
    }

    for (double x = belt.left() - kPattern + offset;
         x < belt.right() + kPattern;
         x += kPattern) {
        const double centerY = belt.center().y();
        const double direction = speed_ >= 0.0 ? 1.0 : -1.0;

        painter.drawLine(
            QPointF(x - 6.0 * direction, centerY - 5.0),
            QPointF(x, centerY));
        painter.drawLine(
            QPointF(x, centerY),
            QPointF(x - 6.0 * direction, centerY + 5.0));
    }

    painter.setClipping(false);
    painter.setPen(QPen(QColor(210, 220, 225), 2.0));
    painter.setBrush(QColor(60, 68, 76));
    painter.drawEllipse(QPointF(body.left() + 8.0, body.center().y()), 4.0, 4.0);
    painter.drawEllipse(QPointF(body.right() - 8.0, body.center().y()), 4.0, 4.0);
    painter.restore();
}
