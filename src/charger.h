#pragma once

#include "entities.h"

#include <QPainter>
#include <QRectF>
#include <QString>
#include <QVector>
#include <QVector2D>

enum class ChargerState {
    Patrol,
    Warning,
    Charging,
    Stunned,
    Recovering
};

struct ChargerConfig {
    QPointF position;
    double leftLimit = 0.0;
    double rightLimit = 0.0;
    double vision = 500.0;
    double warningTime = 0.6;
    double chargeSpeed = 520.0;
    double stunTime = 2.5;
    int health = 5;
    int contactDamage = 1;
};

class Charger {
public:
    explicit Charger(const ChargerConfig &config = {});

    void update(
        double dt,
        const QPointF &player,
        const QVector<QRectF> &solidGeometry,
        const QVector<QRectF> &oneWayPlatforms,
        bool inWater,
        double conveyorSpeed,
        double iceFriction);

    void damage(int amount, const QPointF &source);
    void applyExplosion(const QPointF &center, int damage);
    void launch(const QVector2D &impulse);
    void stun();
    void draw(QPainter &painter, double cameraX, double time) const;

    QRectF rect() const;
    QVector2D velocity() const;
    bool alive() const;
    bool charging() const;
    int direction() const;
    int contactDamage() const;
    int reward() const;
    QString debugText() const;

private:
    void enterState(ChargerState state, double duration = 0.0);
    QString stateName() const;

    QRectF rect_;
    QVector2D velocity_;
    ChargerState state_ = ChargerState::Patrol;

    double leftLimit_ = 0.0;
    double rightLimit_ = 0.0;
    double vision_ = 500.0;
    double warningTime_ = 0.6;
    double chargeSpeed_ = 520.0;
    double stunTime_ = 2.5;
    double stateTimer_ = 0.0;
    double animationTime_ = 0.0;
    double hurtFlash_ = 0.0;

    int health_ = 5;
    int contactDamage_ = 1;
    int direction_ = 1;
};
