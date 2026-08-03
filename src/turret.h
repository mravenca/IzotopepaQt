#pragma once

#include "enemybrain.h"
#include "entities.h"

#include <QPainter>

struct TurretConfig {
    QPointF position;
    QString mount = "floor";
    int direction = -1;
    double vision = 400.0;
    double visionAngle = 100.0;
    double rotationSpeed = 180.0;
    double projectileSpeed = 420.0;
    int health = 4;
    int burst = 3;
    double reload = 2.2;
};

class Turret {
public:
    explicit Turret(const TurretConfig &config = {});

    void update(
        double dt,
        const QPointF &player,
        QVector<Projectile> &shots);

    void damage(int amount, const QPointF &source);
    void applyExplosion(const QPointF &center, int damage);
    void draw(QPainter &painter, double cameraX, double time) const;

    QRectF rect() const;
    bool alive() const;
    int reward() const;
    QString debugText() const;

private:
    bool playerInVisionCone(const QPointF &player) const;
    void fireOneShot(const QPointF &player, QVector<Projectile> &shots);

    QRectF rect_;
    EnemyBrain brain_;
    QString mount_ = "floor";

    double baseAngle_ = 180.0;
    double aimAngle_ = 180.0;
    double visionAngle_ = 100.0;
    double rotationSpeed_ = 180.0;
    double projectileSpeed_ = 420.0;
    double hurtFlash_ = 0.0;
    double recoil_ = 0.0;
    double muzzleFlash_ = 0.0;
    double burstTimer_ = 0.0;

    int health_ = 4;
    int burstSize_ = 3;
    int burstRemaining_ = 0;
};
