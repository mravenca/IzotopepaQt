#include "environmentvolume.h"
#include <cmath>

#include <QLinearGradient>
#include <QPainterPath>
#include <QtMath>
#include <algorithm>

IceSurface::IceSurface(QRectF rect, double friction)
    : rect_(rect),
      friction_(std::clamp(friction, 0.01, 1.0))
{
}

const QRectF &IceSurface::rect() const { return rect_; }
double IceSurface::friction() const { return friction_; }

bool IceSurface::supports(const QRectF &object) const
{
    const QRectF feet(
        object.left() + 4.0,
        object.bottom() - 4.0,
        std::max(1.0, object.width() - 8.0),
        8.0);

    return feet.intersects(rect_)
        && object.bottom() <= rect_.bottom() + 7.0;
}

void IceSurface::draw(QPainter &painter, double cameraX, double time) const
{
    const QRectF rect = rect_.translated(-cameraX, 0.0);

    QLinearGradient fill(rect.topLeft(), rect.bottomLeft());
    fill.setColorAt(0.0, QColor(225, 250, 255, 235));
    fill.setColorAt(0.35, QColor(150, 220, 245, 225));
    fill.setColorAt(1.0, QColor(72, 150, 205, 235));

    painter.setPen(QPen(QColor(225, 250, 255), 2.0));
    painter.setBrush(fill);
    painter.drawRoundedRect(rect, 4.0, 4.0);

    painter.setPen(QPen(QColor(255, 255, 255, 165), 2.0));
    const double offset = std::fmod(time * 22.0, 44.0);
    for (double x = rect.left() - 40.0 + offset;
         x < rect.right();
         x += 44.0) {
        painter.drawLine(
            QPointF(x, rect.top() + 4.0),
            QPointF(x + 24.0, rect.top() + 4.0));
    }
}

WaterZone::WaterZone(QRectF rect, double buoyancy, double drag)
    : rect_(rect),
      buoyancy_(std::clamp(buoyancy, 0.0, 1.5)),
      drag_(std::clamp(drag, 0.0, 1.0))
{
}

const QRectF &WaterZone::rect() const { return rect_; }
double WaterZone::buoyancy() const { return buoyancy_; }
double WaterZone::drag() const { return drag_; }
bool WaterZone::contains(const QPointF &point) const { return rect_.contains(point); }
bool WaterZone::overlaps(const QRectF &object) const { return rect_.intersects(object); }

double WaterZone::submergedFraction(const QRectF &object) const
{
    const QRectF overlap = rect_.intersected(object);
    if (overlap.isEmpty() || object.height() <= 0.0) {
        return 0.0;
    }
    return std::clamp(overlap.height() / object.height(), 0.0, 1.0);
}

void WaterZone::drawBack(QPainter &painter, double cameraX, double time) const
{
    const QRectF rect = rect_.translated(-cameraX, 0.0);
    QLinearGradient water(rect.topLeft(), rect.bottomLeft());
    water.setColorAt(0.0, QColor(75, 190, 235, 105));
    water.setColorAt(1.0, QColor(20, 85, 170, 155));
    painter.fillRect(rect, water);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(205, 245, 255, 95));
    for (int i = 0; i < 7; ++i) {
        const double phase = time * (18.0 + i * 1.5) + i * 41.0;
        const double x = rect.left() + std::fmod(phase, std::max(1.0, rect.width()));
        const double y = rect.bottom() - std::fmod(time * (24.0 + i * 3.0) + i * 53.0,
                                               std::max(1.0, rect.height()));
        const double radius = 2.0 + (i % 3);
        painter.drawEllipse(QPointF(x, y), radius, radius);
    }
}

void WaterZone::drawFront(QPainter &painter, double cameraX, double time) const
{
    const QRectF rect = rect_.translated(-cameraX, 0.0);
    painter.setPen(QPen(QColor(195, 245, 255, 220), 3.0));

    QPainterPath wave;
    wave.moveTo(rect.left(), rect.top());
    for (double x = rect.left(); x <= rect.right(); x += 8.0) {
        const double y = rect.top() + qSin(time * 3.2 + x * 0.045) * 3.0;
        wave.lineTo(x, y);
    }
    painter.drawPath(wave);

    painter.setPen(QPen(QColor(80, 185, 235, 100), 1.0));
    painter.drawRect(rect);
}
