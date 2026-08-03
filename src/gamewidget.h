#pragma once

#include "developeroptions.h"
#include "world.h"

#include <QElapsedTimer>
#include <QTimer>
#include <QWidget>

class GameWidget : public QWidget
{
public:
    explicit GameWidget(
        const DeveloperOptions &options = {},
        QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *) override;
    void keyPressEvent(QKeyEvent *) override;
    void keyReleaseEvent(QKeyEvent *) override;

private:
    enum class Mode { Menu, Playing, Paused, Help, Settings, GameOver, Complete };

    void loop();
    void startLevel(int level);
    void startLevelFile(const QString &fileName);
    void reloadCurrentLevel();
    void reloadExternalLevel();
    void restartCurrentLevel();
    void switchCampaignLevel(int delta);
    void showDeveloperStatus(const QString &text);
    void updateWindowTitle();
    void updateMode();
    void drawBackground(QPainter &painter);
    void drawPlatforms(QPainter &painter);
    void drawHud(QPainter &painter);
    void drawDebugOverlay(QPainter &painter);
    void drawMenu(QPainter &painter, const QString &title, const QStringList &items);
    void saveSettings();

    static constexpr int W = 960;
    static constexpr int H = 640;

    QTimer timer_;
    QElapsedTimer elapsed_;
    SpriteSheet playerSheet_;
    SpriteSheet enemySheet_;
    World world_;
    Camera camera_;
    Mode mode_ = Mode::Menu;

    bool left_ = false;
    bool right_ = false;
    bool up_ = false;
    bool down_ = false;
    bool debugOverlay_ = false;
    DeveloperOptions developerOptions_;
    QString directLevelFile_;
    QString developerStatus_;
    double developerStatusTime_ = 0.0;

    int menuIndex_ = 0;
    int unlocked_ = 0;
    double fps_ = 0.0;
    double sceneTime_ = 0.0;
};
