#pragma once

#include <QPainter>
#include <QRectF>

class IceSurface
{
public:
    IceSurface(QRectF rect = {}, double friction = 0.08);

    const QRectF &rect() const;
    double friction() const;
    bool supports(const QRectF &object) const;
    void draw(QPainter &painter, double cameraX, double time) const;

private:
    QRectF rect_;
    double friction_ = 0.08;
};

class WaterZone
{
public:
    WaterZone(QRectF rect = {}, double buoyancy = 0.72, double drag = 0.55);

    const QRectF &rect() const;
    double buoyancy() const;
    double drag() const;
    bool contains(const QPointF &point) const;
    bool overlaps(const QRectF &object) const;
    double submergedFraction(const QRectF &object) const;
    void drawBack(QPainter &painter, double cameraX, double time) const;
    void drawFront(QPainter &painter, double cameraX, double time) const;

private:
    QRectF rect_;
    double buoyancy_ = 0.72;
    double drag_ = 0.55;
};
