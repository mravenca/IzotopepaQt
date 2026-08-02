#pragma once

#include <QPainter>
#include <QRectF>

class Conveyor {
public:
    Conveyor() = default;
    Conveyor(const QRectF &rect, double speed);

    void update(double dt);
    void draw(QPainter &painter, double cameraX) const;

    QRectF rect() const;
    QRectF surfaceZone() const;
    double speed() const;

private:
    QRectF rect_;
    double speed_ = 120.0;
    double animationOffset_ = 0.0;
};
