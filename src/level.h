#pragma once
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QString>
#include <QVector>
struct EnemySpawn{QString kind;QPointF position;double leftLimit=0,rightLimit=0;};
struct MovingSpawn{QRectF rect;double minX=0,maxX=0,speedX=0,speedY=0;};
struct PickupSpawn{QString kind;QPointF position;};
struct DoorSpawn{QString key;QRectF rect;};
struct SwitchSpawn{QString key;QPointF position;};
struct KeySpawn{QString key;QPointF position;};
struct CrateSpawn{QPointF position;QString drop="none";int hp=1;};
struct BarrelSpawn{QPointF position;double radius=150;int damage=3;};
class Level{
public: bool load(const QString&);QString name()const;QSizeF worldSize()const;QPointF playerSpawn()const;
 const QVector<QRectF>&platforms()const;const QVector<MovingSpawn>&moving()const;const QVector<QRectF>&ladders()const;const QVector<QRectF>&spikes()const;
 const QVector<EnemySpawn>&enemies()const;const QVector<QPointF>&coins()const;const QVector<PickupSpawn>&pickups()const;const QVector<DoorSpawn>&doors()const;
 const QVector<SwitchSpawn>&switches()const;const QVector<CrateSpawn>&crates()const;const QVector<BarrelSpawn>&barrels()const;const QVector<KeySpawn>&keys()const;QPointF checkpoint()const;QRectF goal()const;
private: QString name_="Level";QSizeF world_{960,640};QPointF spawn_{80,450},checkpoint_{-1,-1};QVector<QRectF>platforms_,ladders_,spikes_;QVector<MovingSpawn>moving_;QVector<EnemySpawn>enemies_;QVector<QPointF>coins_;QVector<PickupSpawn>pickups_;QVector<DoorSpawn>doors_;QVector<SwitchSpawn>switches_;QVector<KeySpawn>keys_;QVector<CrateSpawn>crates_;QVector<BarrelSpawn>barrels_;QRectF goal_{880,470,55,100};};
