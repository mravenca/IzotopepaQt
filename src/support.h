#pragma once
#include <QImage>
#include <QRect>
#include <QRectF>
#include <QVector>
#include <QVector2D>
class SpriteSheet { public: SpriteSheet()=default; SpriteSheet(QImage,int,int); bool valid()const; QRect frame(int,int)const; const QImage& image()const; private: QImage image_; int fw_=0,fh_=0; };
QImage loadTransparentImage(const QString&,int threshold=250);
struct MoveResult { bool onGround=false, hitWall=false, hitCeiling=false; };
MoveResult moveAndCollide(QRectF&,QVector2D&,double,const QVector<QRectF>&);
class Camera { public: void configure(double,double); void update(double,double); void reset(double=0); double x()const; private: double x_=0,vw_=960,ww_=960; };
bool intersectsAny(const QRectF&,const QVector<QRectF>&);
