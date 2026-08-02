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
MoveResult moveAndCollideOneWay(
    QRectF &rect,
    QVector2D &velocity,
    double dt,
    const QVector<QRectF> &solids,
    const QVector<QRectF> &oneWayPlatforms,
    bool ignoreOneWay = false);
class Camera {
public:
    void configure(double viewportWidth, double worldWidth);

    // movementHint is -1 for left, 0 for idle and +1 for right.
    void update(double targetX, double movementHint, double dt);

    void reset(double x = 0);
    double x() const;
    double lookAhead() const;

private:
    double x_ = 0;
    double vw_ = 960;
    double ww_ = 960;
    double lookAhead_ = 0;
};
bool intersectsAny(const QRectF&,const QVector<QRectF>&);
