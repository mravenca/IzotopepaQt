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
void Camera::configure(double a,double b){vw_=a;ww_=std::max(a,b);}void Camera::update(double tx,double dt){double d=tx-vw_*.42;x_+=(d-x_)*std::min(1.0,dt*6);x_=std::clamp(x_,0.0,ww_-vw_);}void Camera::reset(double x){x_=std::clamp(x,0.0,std::max(0.0,ww_-vw_));}double Camera::x()const{return x_;}
bool intersectsAny(const QRectF&r,const QVector<QRectF>&v){for(const auto&a:v)if(r.intersects(a))return true;return false;}
