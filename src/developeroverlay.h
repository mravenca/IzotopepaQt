#pragma once

#include <QPointF>
#include <QString>
#include <QVector2D>

class QPainter;

struct EnemyStatistics
{
    int legacy = 0;
    int drones = 0;
    int turrets = 0;
    int chargers = 0;
    int shields = 0;

    int total() const
    {
        return legacy + drones + turrets + chargers + shields;
    }
};

struct DeveloperOverlayData
{
    double fps = 0.0;
    double frameTimeMs = 0.0;

    int levelNumber = 0;
    QString levelName;
    QString levelSource;
    bool developerMode = false;
    bool progressSaving = true;
    bool godMode = false;

    QPointF playerPosition;
    QVector2D playerVelocity;
    bool grounded = false;
    bool climbing = false;
    bool swimming = false;
    bool onIce = false;
    bool invulnerable = false;
    int health = 0;
    int ammo = 0;
    int score = 0;
    int keys = 0;

    EnemyStatistics enemies;
    int projectiles = 0;
    int particles = 0;

    double cameraX = 0.0;
    double cameraLookAhead = 0.0;
    double cameraShake = 0.0;
    bool hitStop = false;

    QString selectedEnemy;
};

class DeveloperOverlay
{
public:
    void draw(
        QPainter &painter,
        const DeveloperOverlayData &data,
        int viewportWidth,
        int viewportHeight,
        bool compact) const;
};
