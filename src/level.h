#pragma once

#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <QVector>

struct EnemySpawn {
    QString kind;
    QPointF position;
    double leftLimit = 0;
    double rightLimit = 0;
};

struct MovingSpawn {
    QRectF rect;
    double minX = 0;
    double maxX = 0;
    double speedX = 0;
    double speedY = 0;
};

struct PickupSpawn {
    QString kind;
    QPointF position;
};

struct DoorSpawn {
    QString key;
    QRectF rect;
};

struct SwitchSpawn {
    QString key;
    QPointF position;
};

struct KeySpawn {
    QString key;
    QPointF position;
};

struct CrateSpawn {
    QPointF position;
    QString drop = "none";
    int hp = 1;
};

struct BarrelSpawn {
    QPointF position;
    double radius = 150;
    int damage = 3;
};

struct PushBoxSpawn {
    QPointF position;
    double width = 48;
    double height = 48;
};

struct PressurePlateSpawn {
    QPointF position;
    double width = 72;
    double height = 18;
    QString target;
    double requiredWeight = 1.0;
};


struct ConveyorSpawn {
    QRectF rect;
    double speed = 120.0;
};

struct OneWayPlatformSpawn {
    QRectF rect;
};


struct FallingPlatformSpawn {
    QRectF rect;
    QString material = "stone";
    double confirmationTime = 0.15;
    double fallDelay = -1.0;
    double respawnDelay = 4.0;
};

struct JumpPadSpawn {
    QPointF position;
    double width = 72;
    double height = 20;
    double strength = 900;
    double horizontalImpulse = 0;
    double launchDelay = 0.08;
    double cooldown = 0.25;
};

class Level {
public:
    bool load(const QString &fileName);

    QString name() const;
    QSizeF worldSize() const;
    QPointF playerSpawn() const;

    const QVector<QRectF> &platforms() const;
    const QVector<MovingSpawn> &moving() const;
    const QVector<QRectF> &ladders() const;
    const QVector<QRectF> &spikes() const;
    const QVector<EnemySpawn> &enemies() const;
    const QVector<QPointF> &coins() const;
    const QVector<PickupSpawn> &pickups() const;
    const QVector<DoorSpawn> &doors() const;
    const QVector<SwitchSpawn> &switches() const;
    const QVector<CrateSpawn> &crates() const;
    const QVector<BarrelSpawn> &barrels() const;
    const QVector<PushBoxSpawn> &pushBoxes() const;
    const QVector<JumpPadSpawn> &jumpPads() const;
    const QVector<PressurePlateSpawn> &pressurePlates() const;
    const QVector<ConveyorSpawn> &conveyors() const;
    const QVector<OneWayPlatformSpawn> &oneWayPlatforms() const;
    const QVector<FallingPlatformSpawn> &fallingPlatforms() const;
    const QVector<KeySpawn> &keys() const;

    QPointF checkpoint() const;
    QRectF goal() const;

private:
    QString name_ = "Level";
    QSizeF world_ {960, 640};
    QPointF spawn_ {80, 450};
    QPointF checkpoint_ {-1, -1};

    QVector<QRectF> platforms_;
    QVector<QRectF> ladders_;
    QVector<QRectF> spikes_;
    QVector<MovingSpawn> moving_;
    QVector<EnemySpawn> enemies_;
    QVector<QPointF> coins_;
    QVector<PickupSpawn> pickups_;
    QVector<DoorSpawn> doors_;
    QVector<SwitchSpawn> switches_;
    QVector<KeySpawn> keys_;
    QVector<CrateSpawn> crates_;
    QVector<BarrelSpawn> barrels_;
    QVector<PushBoxSpawn> pushBoxes_;
    QVector<JumpPadSpawn> jumpPads_;
    QVector<PressurePlateSpawn> pressurePlates_;
    QVector<ConveyorSpawn> conveyors_;
    QVector<OneWayPlatformSpawn> oneWayPlatforms_;
    QVector<FallingPlatformSpawn> fallingPlatforms_;

    QRectF goal_ {880, 470, 55, 100};
};
