#pragma once
#include "entities.h"
#include "level.h"
#include <QHash>
#include <QSet>
struct MovingPlatform{QRectF rect;double minX=0,maxX=0,speedX=0,speedY=0,dir=1;QPointF delta;};
struct Coin{QRectF rect;bool collected=false;double phase=0;};
struct Pickup{QString kind;QRectF rect;bool taken=false;};
struct Door{QString key;QRectF rect;bool open=false;};
struct SwitchObj{QString key;QRectF rect;bool active=false;};
struct KeyObj{QString key;QRectF rect;bool taken=false;};
class World{
public: World(const SpriteSheet*,const SpriteSheet*);bool loadLevel(int);void resetFromCheckpoint();void update(double);void draw(QPainter&,double)const;Player&player();const Player&player()const;double width()const;QString levelName()const;int levelIndex()const;bool completed()const;bool gameOver()const;void setInput(bool,bool,bool,bool);void jump();void stopJump();void shoot();void interact();void toggleSound();bool soundEnabled()const;QString message()const;
private:void rebuildCollision();void explode(QPointF,QColor);void checkpoint();void beep()const;
 const SpriteSheet*playerSheet_;const SpriteSheet*enemySheet_;Level level_;Player player_;QVector<Enemy>enemies_;QVector<MovingPlatform>moving_;QVector<Coin>coins_;QVector<Pickup>pickups_;QVector<Door>doors_;QVector<SwitchObj>switches_;QVector<KeyObj>keys_;QVector<Projectile>projectiles_;QVector<Particle>particles_;QVector<QRectF>collision_;QPointF respawn_;int levelIndex_=0;bool completed_=false,gameOver_=false,sound_=true;QString message_;double messageTime_=0;double shakeTime_=0,shakeStrength_=0;
};
