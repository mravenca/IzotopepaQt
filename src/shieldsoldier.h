#pragma once

#include "entities.h"

#include <QPainter>
#include <QRectF>
#include <QString>
#include <QVector>
#include <QVector2D>

enum class ShieldState {
    Patrol,
    Advance,
    Defend,
    Attack,
    Staggered
};

enum class ShieldBulletResult {
    Missed,
    Blocked,
    Damaged,
    Destroyed
};

struct ShieldSoldierConfig {
    QPointF position;
    double leftLimit = 0.0;
    double rightLimit = 0.0;
    double vision = 420.0;
    double speed = 95.0;
    double shieldAngle = 130.0;
    double fireCooldown = 2.4;
    int health = 6;
};

class ShieldSoldier {
public:
    explicit ShieldSoldier(const ShieldSoldierConfig &config = {});

    void update(
        double dt,
        const QPointF &player,
        const QVector<QRectF> &solidGeometry,
        const QVector<QRectF> &oneWayPlatforms,
        bool inWater,
        double conveyorSpeed,
        double iceFriction,
        QVector<Projectile> &shots);

    ShieldBulletResult receiveBullet(int amount, const QPointF &source);
    void applyExplosion(const QPointF &center, int damage);
    void launch(const QVector2D &impulse);
    void draw(QPainter &painter, double cameraX, double time) const;

    QRectF rect() const;
    QVector2D velocity() const;
    bool alive() const;
    int reward() const;
    QString debugText() const;

private:
    bool sourceInShieldArc(const QPointF &source) const;
    void fire(const QPointF &player, QVector<Projectile> &shots);
    void enterState(ShieldState state, double duration = 0.0);
    QString stateName() const;

    QRectF rect_;
    QVector2D velocity_;
    ShieldState state_ = ShieldState::Patrol;

    double leftLimit_ = 0.0;
    double rightLimit_ = 0.0;
    double vision_ = 420.0;
    double moveSpeed_ = 95.0;
    double shieldAngle_ = 130.0;
    double fireCooldownTime_ = 2.4;
    double fireCooldown_ = 0.0;
    double stateTimer_ = 0.0;
    double animationTime_ = 0.0;
    double hurtFlash_ = 0.0;
    double shieldFlash_ = 0.0;
    double muzzleFlash_ = 0.0;

    int health_ = 6;
    int direction_ = 1;
};
