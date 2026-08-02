#include "support.h"
#include <QColor>
#include <QDebug>
#include <algorithm>
#include <utility>
SpriteSheet::SpriteSheet(QImage i,int fw,int fh):image_(std::move(i)),fw_(fw),fh_(fh){}
bool SpriteSheet::valid()const{return !image_.isNull()&&fw_>0&&fh_>0;}
QRect SpriteSheet::frame(int r,int c)const{return QRect(c*fw_,r*fh_,fw_,fh_);}
const QImage& SpriteSheet::image()const{return image_;}
QImage loadTransparentImage(const QString&p,int t){QImage i(p);if(i.isNull()){qWarning()<<"Cannot load"<<p;return i;}i=i.convertToFormat(QImage::Format_ARGB32);for(int y=0;y<i.height();++y){auto*l=reinterpret_cast<QRgb*>(i.scanLine(y));for(int x=0;x<i.width();++x){QColor c=QColor::fromRgba(l[x]);if(c.red()>=t&&c.green()>=t&&c.blue()>=t)l[x]=qRgba(255,255,255,0);}}return i;}
MoveResult moveAndCollide(QRectF&r,QVector2D&v,double dt,const QVector<QRectF>&ps){MoveResult z;r.translate(v.x()*dt,0);for(const auto&p:ps)if(r.intersects(p)){if(v.x()>0)r.moveRight(p.left());else if(v.x()<0)r.moveLeft(p.right());v.setX(0);z.hitWall=true;}r.translate(0,v.y()*dt);for(const auto&p:ps)if(r.intersects(p)){if(v.y()>0){r.moveBottom(p.top());z.onGround=true;}else if(v.y()<0){r.moveTop(p.bottom());z.hitCeiling=true;}v.setY(0);}return z;}
void Camera::configure(double viewportWidth, double worldWidth)
{
    vw_ = viewportWidth;
    ww_ = std::max(viewportWidth, worldWidth);
}

void Camera::update(
    double targetX,
    double movementHint,
    double dt)
{
    const double maxCameraX = std::max(0.0, ww_ - vw_);

    const double desiredLookAhead =
        std::clamp(movementHint, -1.0, 1.0) * 115.0;

    const double lookSpeed =
        qFuzzyIsNull(movementHint) ? 3.0 : 5.5;

    lookAhead_ +=
        (desiredLookAhead - lookAhead_)
        * std::min(1.0, dt * lookSpeed);

    const double screenTarget =
        targetX - x_ + lookAhead_;

    const double deadZoneLeft = vw_ * 0.36;
    const double deadZoneRight = vw_ * 0.58;

    double desiredCameraX = x_;

    if (screenTarget < deadZoneLeft) {
        desiredCameraX =
            targetX + lookAhead_ - deadZoneLeft;
    } else if (screenTarget > deadZoneRight) {
        desiredCameraX =
            targetX + lookAhead_ - deadZoneRight;
    }

    desiredCameraX =
        std::clamp(desiredCameraX, 0.0, maxCameraX);

    const double followFactor =
        1.0 - std::exp(-8.0 * dt);

    x_ +=
        (desiredCameraX - x_)
        * followFactor;

    x_ = std::clamp(x_, 0.0, maxCameraX);
}

void Camera::reset(double x)
{
    x_ = std::clamp(
        x,
        0.0,
        std::max(0.0, ww_ - vw_));
    lookAhead_ = 0.0;
}

double Camera::x() const
{
    return x_;
}

double Camera::lookAhead() const
{
    return lookAhead_;
}
bool intersectsAny(const QRectF&r,const QVector<QRectF>&v){for(const auto&a:v)if(r.intersects(a))return true;return false;}
