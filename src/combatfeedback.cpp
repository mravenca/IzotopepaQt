#include "combatfeedback.h"

#include <QRandomGenerator>
#include <QtMath>

#include <algorithm>

namespace {
QColor defaultColor(CombatImpact impact)
{
    switch (impact) {
    case CombatImpact::Bullet:
        return QColor(255, 220, 70);
    case CombatImpact::Metal:
        return QColor(255, 175, 55);
    case CombatImpact::Shield:
        return QColor(105, 220, 255);
    case CombatImpact::Explosion:
        return QColor(255, 110, 30);
    case CombatImpact::EnemyDeath:
        return QColor(255, 75, 45);
    }

    return Qt::yellow;
}
}

void CombatFeedback::appendImpact(
    QVector<Particle> &particles,
    const QPointF &position,
    CombatImpact impact,
    double intensity,
    QColor accent)
{
    intensity = std::clamp(intensity, 0.25, 3.0);
    const QColor base = accent.isValid() ? accent : defaultColor(impact);

    int count = 7;
    double minimumSpeed = 55.0;
    double speedRange = 130.0;
    double gravityBias = -0.15;

    if (impact == CombatImpact::Shield) {
        count = 12;
        minimumSpeed = 110.0;
        speedRange = 190.0;
        gravityBias = -0.35;
    } else if (impact == CombatImpact::Explosion) {
        count = 18;
        minimumSpeed = 100.0;
        speedRange = 260.0;
        gravityBias = -0.25;
    } else if (impact == CombatImpact::EnemyDeath) {
        count = 24;
        minimumSpeed = 90.0;
        speedRange = 300.0;
        gravityBias = -0.30;
    }

    count = std::max(3, static_cast<int>(count * intensity));

    for (int index = 0; index < count; ++index) {
        const double angle =
            QRandomGenerator::global()->generateDouble() * 6.28318530718;
        const double speed =
            (minimumSpeed
             + QRandomGenerator::global()->generateDouble() * speedRange)
            * intensity;

        QVector2D velocity(
            static_cast<float>(qCos(angle) * speed),
            static_cast<float>((qSin(angle) + gravityBias) * speed));

        QColor color = base;
        if (index % 4 == 0) {
            color = color.lighter(145);
        } else if (index % 5 == 0) {
            color = color.darker(135);
        }

        particles << Particle {
            position,
            velocity,
            0.25
                + QRandomGenerator::global()->generateDouble()
                    * (impact == CombatImpact::EnemyDeath ? 0.75 : 0.45),
            2.0
                + QRandomGenerator::global()->generateDouble()
                    * (impact == CombatImpact::Explosion
                           || impact == CombatImpact::EnemyDeath
                       ? 6.0
                       : 3.5),
            color
        };
    }
}
