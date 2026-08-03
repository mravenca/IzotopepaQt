#pragma once

#include <QPointF>
#include <QString>

enum class EnemyState {
    Patrol,
    Alert,
    Attack,
    Search,
    Return
};

class EnemyBrain {
public:
    EnemyBrain(double visionRange = 420.0, double reloadTime = 2.0);

    void reset();
    void update(double dt, const QPointF &self, const QPointF &target);
    void consumeShot();

    EnemyState state() const;
    QString stateName() const;
    bool seesTarget() const;
    bool canFire() const;
    double fireCooldown() const;
    double visionRange() const;
    QPointF lastKnownTarget() const;

private:
    EnemyState state_ = EnemyState::Patrol;
    double visionRange_ = 420.0;
    double reloadTime_ = 2.0;
    double fireCooldown_ = 0.0;
    double targetMemory_ = 0.0;
    bool seesTarget_ = false;
    QPointF lastKnownTarget_;
};
