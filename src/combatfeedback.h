#pragma once

#include "entities.h"

#include <QColor>
#include <QPointF>
#include <QVector>

enum class CombatImpact {
    Bullet,
    Metal,
    Shield,
    Explosion,
    EnemyDeath
};

class CombatFeedback {
public:
    static void appendImpact(
        QVector<Particle> &particles,
        const QPointF &position,
        CombatImpact impact,
        double intensity = 1.0,
        QColor accent = QColor());
};
