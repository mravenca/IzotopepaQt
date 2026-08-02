#pragma once
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <QVector>

struct EditorMovingPlatform {
    QRectF rect;
    double minX = 0;
    double maxX = 0;
    double speedX = 0;
    double speedY = 0;
};

struct EditorEnemy {
    QString kind;
    QPointF position;
    double left = 0;
    double right = 0;
};

struct EditorPickup {
    QString kind;
    QPointF position;
};

struct EditorNamedPoint {
    QString key;
    QPointF position;
};

struct EditorDoor {
    QString key;
    QRectF rect;
};

class LevelDocument
{
public:
    bool load(const QString &fileName, QString *error = nullptr);

    QString fileName() const { return fileName_; }
    QString name() const { return name_; }
    QSizeF worldSize() const { return worldSize_; }
    QPointF playerSpawn() const { return playerSpawn_; }

    const QVector<QRectF> &platforms() const { return platforms_; }
    const QVector<EditorMovingPlatform> &movingPlatforms() const { return moving_; }
    const QVector<QRectF> &ladders() const { return ladders_; }
    const QVector<QRectF> &spikes() const { return spikes_; }
    const QVector<EditorEnemy> &enemies() const { return enemies_; }
    const QVector<QPointF> &coins() const { return coins_; }
    const QVector<EditorPickup> &pickups() const { return pickups_; }
    const QVector<EditorNamedPoint> &keys() const { return keys_; }
    const QVector<EditorDoor> &doors() const { return doors_; }
    const QVector<EditorNamedPoint> &switches() const { return switches_; }

    QPointF checkpoint() const { return checkpoint_; }
    QRectF goal() const { return goal_; }

private:
    QString fileName_;
    QString name_ = "Untitled";
    QSizeF worldSize_ {960, 640};
    QPointF playerSpawn_ {80, 450};

    QVector<QRectF> platforms_;
    QVector<EditorMovingPlatform> moving_;
    QVector<QRectF> ladders_;
    QVector<QRectF> spikes_;
    QVector<EditorEnemy> enemies_;
    QVector<QPointF> coins_;
    QVector<EditorPickup> pickups_;
    QVector<EditorNamedPoint> keys_;
    QVector<EditorDoor> doors_;
    QVector<EditorNamedPoint> switches_;

    QPointF checkpoint_ {-1, -1};
    QRectF goal_ {880, 470, 55, 100};
};
