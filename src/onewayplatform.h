#pragma once

#include <QPainter>
#include <QRectF>

class OneWayPlatform {
public:
    explicit OneWayPlatform(const QRectF &rect = {});

    const QRectF &rect() const;
    void draw(QPainter &painter, double cameraX) const;

private:
    QRectF rect_;
};
