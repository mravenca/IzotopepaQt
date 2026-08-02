#pragma once

#include "entities.h"
#include "level.h"
#include "jumppad.h"
#include "pressureplate.h"
#include "conveyor.h"
#include "worldevent.h"

#include <QPointF>
#include <QRectF>
#include <QString>
#include <QVector>

struct MovingPlatform {
    QRectF rect;
    double minX = 0;
    double maxX = 0;
    double speedX = 0;
    double speedY = 0;
    double dir = 1;
    QPointF delta;
};

struct Coin {
    QRectF rect;
    bool collected = false;
    double phase = 0;
};

struct Pickup {
    QString kind;
    QRectF rect;
    bool taken = false;
};

struct Door {
    QString key;
    QRectF rect;
    bool open = false;
    bool latchedOpen = false;
    bool signalActive = false;
};

struct SwitchObj {
    QString key;
    QRectF rect;
    bool active = false;
};

struct KeyObj {
    QString key;
    QRectF rect;
    bool taken = false;
};

struct Crate {
    QRectF rect;
    QString drop = "none";
    int hp = 1;
    bool alive = true;
};

struct Barrel {
    QRectF rect;
    double radius = 150;
    int damage = 3;
    double fuse = -1;
    bool alive = true;
};

struct PushBox {
    QRectF rect;
    QVector2D velocity;
    bool alive = true;
};

struct ExplosionEvent {
    QPointF center;
    double radius = 150;
    int damage = 3;
};

struct JumpPadActivation {
    bool player = false;
    QVector<int> pushBoxes;
};

class World {
public:
    World(const SpriteSheet *playerSheet, const SpriteSheet *enemySheet);

    bool loadLevel(int index);
    void resetFromCheckpoint();
    void update(double dt);
    void draw(QPainter &painter, double cameraX) const;

    Player &player();
    const Player &player() const;

    double width() const;
    QString levelName() const;
    int levelIndex() const;
    bool completed() const;
    bool gameOver() const;

    void setInput(bool left, bool right, bool up, bool down);
    void jump();
    void stopJump();
    void shoot();
    void interact();

    void toggleSound();
    bool soundEnabled() const;
    QString message() const;

private:
    void rebuildCollision();
    void explode(QPointF position, QColor color);
    void applyExplosion(const ExplosionEvent &event);
    void destroyCrate(Crate &crate);
    void updatePushBoxes(double dt);
    void updateJumpPads(double dt);
    void updatePressurePlates(double dt);
    void updateConveyors(double dt);
    double conveyorSpeedBelow(const QRectF &rect) const;
    void processWorldEvents();
    void refreshDoorStates();
    void launchFromPad(int padIndex);
    QVector<QRectF> pushBoxBlockers(int excludedIndex) const;
    void checkpoint();
    void beep() const;

    const SpriteSheet *playerSheet_ = nullptr;
    const SpriteSheet *enemySheet_ = nullptr;

    Level level_;
    Player player_;

    QVector<Enemy> enemies_;
    QVector<MovingPlatform> moving_;
    QVector<Coin> coins_;
    QVector<Pickup> pickups_;
    QVector<Door> doors_;
    QVector<SwitchObj> switches_;
    QVector<KeyObj> keys_;
    QVector<Crate> crates_;
    QVector<Barrel> barrels_;
    QVector<PushBox> pushBoxes_;
    QVector<JumpPad> jumpPads_;
    QVector<PressurePlate> pressurePlates_;
    QVector<Conveyor> conveyors_;
    WorldEventQueue worldEvents_;
    QVector<JumpPadActivation> jumpPadActivations_;
    QVector<Projectile> projectiles_;
    QVector<Particle> particles_;
    QVector<QRectF> collision_;

    QPointF respawn_;
    int levelIndex_ = 0;

    bool completed_ = false;
    bool gameOver_ = false;
    bool sound_ = true;
    bool inputLeft_ = false;
    bool inputRight_ = false;

    QString message_;
    double messageTime_ = 0;
    double shakeTime_ = 0;
    double shakeStrength_ = 0;
    double animationTime_ = 0;
};
