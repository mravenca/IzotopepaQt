#pragma once

#include "worldevent.h"

#include <QPainter>
#include <QRectF>
#include <QString>

class PressurePlate {
public:
    PressurePlate() = default;
    PressurePlate(
        const QRectF &rect,
        QString target,
        double requiredWeight = 1.0);

    void update(double totalWeight, double dt, WorldEventQueue &events);
    void draw(QPainter &painter, double cameraX) const;

    QRectF rect() const;
    QRectF triggerZone() const;
    QString target() const;
    bool pressed() const;
    double requiredWeight() const;

private:
    QRectF rect_;
    QString target_;
    double requiredWeight_ = 1.0;
    double compression_ = 0.0;
    bool pressed_ = false;
};
