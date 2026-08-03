#include "enemybrain.h"

#include <QtMath>
#include <algorithm>

EnemyBrain::EnemyBrain(double visionRange, double reloadTime)
    : visionRange_(std::max(1.0, visionRange)),
      reloadTime_(std::max(0.1, reloadTime))
{
}

void EnemyBrain::reset()
{
    state_ = EnemyState::Patrol;
    fireCooldown_ = 0.0;
    targetMemory_ = 0.0;
    seesTarget_ = false;
    lastKnownTarget_ = {};
}

void EnemyBrain::update(
    double dt,
    const QPointF &self,
    const QPointF &target)
{
    const QPointF delta = target - self;
    const double distance = qSqrt(
        delta.x() * delta.x() + delta.y() * delta.y());

    updateVisible(dt, self, target, distance <= visionRange_);
}

void EnemyBrain::updateVisible(
    double dt,
    const QPointF &self,
    const QPointF &target,
    bool targetVisible)
{
    Q_UNUSED(self);

    fireCooldown_ = std::max(0.0, fireCooldown_ - dt);
    targetMemory_ = std::max(0.0, targetMemory_ - dt);
    seesTarget_ = targetVisible;

    if (seesTarget_) {
        lastKnownTarget_ = target;
        targetMemory_ = 2.2;
        state_ = fireCooldown_ <= 0.0
            ? EnemyState::Attack
            : EnemyState::Alert;
        return;
    }

    if (targetMemory_ > 0.0) {
        state_ = EnemyState::Search;
    } else if (state_ == EnemyState::Search
               || state_ == EnemyState::Alert
               || state_ == EnemyState::Attack) {
        state_ = EnemyState::Return;
    } else {
        state_ = EnemyState::Patrol;
    }
}

void EnemyBrain::consumeShot()
{
    fireCooldown_ = reloadTime_;
    state_ = EnemyState::Alert;
}

EnemyState EnemyBrain::state() const { return state_; }
bool EnemyBrain::seesTarget() const { return seesTarget_; }
bool EnemyBrain::canFire() const { return seesTarget_ && fireCooldown_ <= 0.0; }
double EnemyBrain::fireCooldown() const { return fireCooldown_; }
double EnemyBrain::visionRange() const { return visionRange_; }
QPointF EnemyBrain::lastKnownTarget() const { return lastKnownTarget_; }

QString EnemyBrain::stateName() const
{
    switch (state_) {
    case EnemyState::Patrol: return "PATROL";
    case EnemyState::Alert: return "ALERT";
    case EnemyState::Attack: return "ATTACK";
    case EnemyState::Search: return "SEARCH";
    case EnemyState::Return: return "RETURN";
    }

    return "UNKNOWN";
}
