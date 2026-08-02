#pragma once
#include "support.h"
#include <QPainter>
#include <QSet>
struct Projectile{QRectF rect;QVector2D velocity;bool hostile=false,alive=true;double life=2.5;};
struct Particle{QPointF pos;QVector2D velocity;double life=.5,size=5;QColor color=Qt::yellow;};
class Player{public:explicit Player(const SpriteSheet *sheet = nullptr);void reset(QPointF,bool full=true);void update(double,const QVector<QRectF>&,const QVector<QRectF>&,double);void setLeft(bool);void setRight(bool);void setUp(bool);void setDown(bool);void jump();bool canShoot()const;Projectile shoot();void damage(double);void heal(int);void addAmmo(int);void addScore(int);void addKey(const QString&);bool hasKey(const QString&)const;QRectF rect()const;QPointF position()const;int health()const;int ammo()const;int score()const;bool dead()const;void draw(QPainter&,double)const;private:const SpriteSheet*sheet_=nullptr;QRectF rect_;QVector2D vel_;bool left_=false,right_=false,up_=false,down_=false,onGround_=false,onLadder_=false;int direction_=1,health_=5,ammo_=20,score_=0;double shootCd_=0,invuln_=0,anim_=0;QSet<QString>keys_;};
class Enemy{public:Enemy(const SpriteSheet*,QString,QPointF,double,double);void update(double,const QVector<QRectF>&,QPointF,QVector<Projectile>&);void damage(int);void draw(QPainter&,double)const;QRectF rect()const;bool alive()const;int reward()const;private:const SpriteSheet*sheet_=nullptr;QString kind_;QRectF rect_;QVector2D vel_;double left_=0,right_=0,speed_=90,shootCd_=1,anim_=0;int direction_=1,frame_=0,hp_=1;};
