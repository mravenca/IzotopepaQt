#pragma once
#include "world.h"
#include <QElapsedTimer>
#include <QTimer>
#include <QWidget>
class GameWidget:public QWidget{public:explicit GameWidget(QWidget*parent=nullptr);protected:void paintEvent(QPaintEvent*)override;void keyPressEvent(QKeyEvent*)override;void keyReleaseEvent(QKeyEvent*)override;private:void loop();enum class Mode{Menu,Playing,Paused,Help,Settings,GameOver,Complete};void startLevel(int);void updateMode();void drawBackground(QPainter&);void drawPlatforms(QPainter&);void drawHud(QPainter&);void drawMenu(QPainter&,const QString&,const QStringList&);void saveSettings();static constexpr int W=960,H=640;QTimer timer_;QElapsedTimer elapsed_;SpriteSheet playerSheet_,enemySheet_;World world_;Camera camera_;Mode mode_=Mode::Menu;bool left_=false,right_=false,up_=false,down_=false;int menuIndex_=0,unlocked_=0;};
