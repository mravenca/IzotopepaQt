#pragma once

#include "enemybrain.h"
#include "entities.h"

#include <QPainter>
#include <QVector>

struct DroneConfig {
    QPointF position;
    QVector<QPointF> patrol;
    double speed = 140.0;
    double vision = 420.0;
    int health = 3;
    int burst = 3;
    double reload = 2.0;
};

class Drone {
public:
    explicit Drone(const DroneConfig &config = {});

    void update(
        double dt,
        const QPointF &player,
        const QVector<QRectF> &solidGeometry,
        QVector<Projectile> &shots);

    void damage(int amount, const QPointF &source);
    void applyExplosion(const QPointF &center, int damage);
    void draw(QPainter &painter, double cameraX, double time) const;

    QRectF rect() const;
    bool alive() const;
    int reward() const;
    QString debugText() const;

private:
    QPointF patrolTarget() const;
    void advanceWaypoint();
    void fireBurst(const QPointF &player, QVector<Projectile> &shots);

    QRectF rect_;
    QVector2D velocity_;
    QVector<QPointF> patrol_;
    EnemyBrain brain_;

    double speed_ = 140.0;
    double hoverPhase_ = 0.0;
    double hurtFlash_ = 0.0;
    double recoil_ = 0.0;
    int health_ = 3;
    int burstSize_ = 3;
    int waypoint_ = 0;
    int direction_ = 1;
};
