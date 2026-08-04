#include "gamewidget.h"
#include "support.h"
#include <QFileInfo>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QRadialGradient>
#include <QSettings>
#include <QtMath>
#include <algorithm>
GameWidget::GameWidget(const DeveloperOptions &options, QWidget *parent)
    : QWidget(parent),
      playerSheet_(loadTransparentImage(":/sprites/izotop.bmp"), 96, 128),
      enemySheet_(loadTransparentImage(":/sprites/potvory.bmp"), 64, 96),
      world_(&playerSheet_, &enemySheet_),
      developerOptions_(options),
      crazyLabLogo_(":/ui/crazy_lab_logo.png")
{
    setFixedSize(W, H);
    setFocusPolicy(Qt::StrongFocus);

    QSettings settings("OpenAI", "Izotopepa");
    unlocked_ = std::clamp(
        settings.value("unlockedLevel", 0).toInt(),
        0,
        9);

    debugOverlay_ = developerOptions_.debugOverlay;
    world_.setDeveloperSession(developerOptions_.developerMode());
    world_.setGodMode(developerOptions_.godMode);

    timer_.setInterval(16);
    connect(&timer_, &QTimer::timeout, this, &GameWidget::loop);
    elapsed_.start();
    timer_.start();

    if (!developerOptions_.levelFile.isEmpty()) {
        startLevelFile(developerOptions_.levelFile);
    } else if (developerOptions_.level >= 0) {
        startLevel(developerOptions_.level);
    } else {
        mode_ = Mode::Splash;
    }

    updateWindowTitle();
}
void GameWidget::startLevel(int level)
{
    directLevelFile_.clear();
    if (!world_.loadLevel(level)) {
        mode_ = Mode::Menu;
        return;
    }

    world_.setDeveloperSession(developerOptions_.developerMode());
    world_.setGodMode(developerOptions_.godMode);
    camera_.configure(W, world_.width());
    camera_.reset();
    mode_ = Mode::Playing;
    left_ = right_ = up_ = down_ = false;
    updateWindowTitle();
}

void GameWidget::startLevelFile(const QString &fileName)
{
    if (!world_.loadLevelFile(fileName)) {
        mode_ = Mode::Menu;
        return;
    }

    directLevelFile_ = fileName;
    world_.setDeveloperSession(developerOptions_.developerMode());
    world_.setGodMode(developerOptions_.godMode);
    camera_.configure(W, world_.width());
    camera_.reset();
    mode_ = Mode::Playing;
    left_ = right_ = up_ = down_ = false;
    updateWindowTitle();
}

void GameWidget::showDeveloperStatus(const QString &text)
{
    developerStatus_ = text;
    developerStatusTime_ = 1.5;
    update();
}

void GameWidget::reloadCurrentLevel()
{
    if (!directLevelFile_.isEmpty()) {
        startLevelFile(directLevelFile_);
        if (mode_ == Mode::Playing) {
            showDeveloperStatus(
                QString("Reloaded %1")
                    .arg(QFileInfo(directLevelFile_).fileName()));
        }
        return;
    }

    const int level = world_.levelIndex();
    if (level < 0) {
        showDeveloperStatus("No campaign level to reload");
        return;
    }

    startLevel(level);
    if (mode_ == Mode::Playing) {
        showDeveloperStatus(
            QString("Reloaded level %1").arg(level + 1));
    }
}

void GameWidget::reloadExternalLevel()
{
    if (directLevelFile_.isEmpty()) {
        showDeveloperStatus(
            "F6 requires a --level-file session");
        return;
    }

    const QString fileName = directLevelFile_;
    startLevelFile(fileName);
    if (mode_ == Mode::Playing) {
        showDeveloperStatus(
            QString("Reloaded %1")
                .arg(QFileInfo(fileName).fileName()));
    }
}

void GameWidget::restartCurrentLevel()
{
    if (!world_.restartLevel()) {
        showDeveloperStatus("Could not restart level");
        return;
    }

    world_.setDeveloperSession(developerOptions_.developerMode());
    world_.setGodMode(developerOptions_.godMode);
    camera_.configure(W, world_.width());
    camera_.reset();
    mode_ = Mode::Playing;
    left_ = right_ = up_ = down_ = false;
    showDeveloperStatus("Restarted current level");
}

void GameWidget::switchCampaignLevel(int delta)
{
    if (!directLevelFile_.isEmpty()) {
        showDeveloperStatus(
            "Level switching is unavailable for --level-file");
        return;
    }

    const int current = std::max(0, world_.levelIndex());
    const int target = std::clamp(current + delta, 0, 9);

    if (target == current) {
        showDeveloperStatus(
            target == 0 ? "Already at level 1"
                        : "Already at level 10");
        return;
    }

    startLevel(target);
    if (mode_ == Mode::Playing) {
        showDeveloperStatus(
            QString("Loaded level %1").arg(target + 1));
    }
}

void GameWidget::updateWindowTitle()
{
    QString title = "Izotopepa Complete Edition";

    if (developerOptions_.developerMode()) {
        title += " — Developer Mode";
    }

    if (!directLevelFile_.isEmpty()) {
        title += QString(" — %1")
            .arg(QFileInfo(directLevelFile_).fileName());
    } else if (mode_ == Mode::Playing && world_.levelIndex() >= 0) {
        title += QString(" — Level %1")
            .arg(world_.levelIndex() + 1);
    }

    setWindowTitle(title);
}
void GameWidget::loop()
{
    const double dt =
        std::min(0.05, elapsed_.restart() / 1000.0);

    sceneTime_ += dt;
    developerStatusTime_ =
        std::max(0.0, developerStatusTime_ - dt);

    if (developerStatusTime_ <= 0.0) {
        developerStatus_.clear();
    }

    if (dt > 0.0) {
        const double instantFps = 1.0 / dt;
        fps_ = fps_ <= 0.0
            ? instantFps
            : fps_ * 0.9 + instantFps * 0.1;
    }

    if (mode_ == Mode::Splash) {
        splashTime_ += dt;
        if (splashTime_ >= 3.2) {
            finishSplash();
        }
    }

    if (mode_ == Mode::Playing) {
        world_.setInput(left_, right_, up_, down_);
        world_.update(dt);

        const double cameraDirection =
            (right_ ? 1.0 : 0.0)
            - (left_ ? 1.0 : 0.0);

        camera_.update(
            world_.player().rect().center().x(),
            cameraDirection,
            dt);
        updateMode();
    }

    update();
}
void GameWidget::updateMode()
{
    if (world_.gameOver()) {
        mode_ = Mode::GameOver;
    } else if (world_.completed()) {
        if (!developerOptions_.developerMode()) {
            unlocked_ = std::max(
                unlocked_,
                std::min(9, world_.levelIndex() + 1));
        }
        mode_ = Mode::Complete;
    }
}
void GameWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (mode_ == Mode::Splash) {
        drawSplash(p);
        return;
    }

    drawBackground(p);

    if (mode_ != Mode::Menu
        && mode_ != Mode::Help
        && mode_ != Mode::Settings) {
        drawPlatforms(p);
        world_.draw(p, camera_.x());
        drawHud(p);
    }

    if (mode_ == Mode::Menu) {
        drawMenu(
            p,
            "IZOTOPEPA",
            {"NEW GAME", "CONTINUE", "CONTROLS", "SETTINGS", "QUIT"});
    } else if (mode_ == Mode::Paused) {
        drawMenu(p, "PAUSED", {"RESUME", "RESTART LEVEL", "MAIN MENU"});
    } else if (mode_ == Mode::Help) {
        drawMenu(
            p,
            "CONTROLS",
            {"A/D or arrows: move",
             "Space/W/Up: jump or climb",
             "S/Down: climb down",
             "F/Ctrl: shoot",
             "E: activate switch",
             "P/Esc: pause",
             "Backspace: return"});
    } else if (mode_ == Mode::Settings) {
        drawMenu(
            p,
            "SETTINGS",
            {QString("Sound: %1").arg(world_.soundEnabled() ? "On" : "Off"),
             "Reset saved progress",
             "Back"});
    } else if (mode_ == Mode::GameOver) {
        drawMenu(
            p,
            "GAME OVER",
            {"RESTART FROM CHECKPOINT", "RESTART LEVEL", "MAIN MENU"});
    } else if (mode_ == Mode::Complete) {
        drawMenu(
            p,
            "LEVEL COMPLETE",
            {world_.levelIndex() < 9 ? "NEXT LEVEL" : "PLAY CAMPAIGN AGAIN",
             "MAIN MENU"});
    }
}

void GameWidget::drawSplash(QPainter &p)
{
    p.fillRect(rect(), QColor(7, 8, 10));

    QRadialGradient glow(QPointF(W / 2.0, H / 2.0), 520.0);
    glow.setColorAt(0.0, QColor(53, 42, 29));
    glow.setColorAt(0.48, QColor(19, 21, 24));
    glow.setColorAt(1.0, QColor(3, 4, 5));
    p.fillRect(rect(), glow);

    double opacity = 1.0;
    if (splashTime_ < 0.65) {
        opacity = splashTime_ / 0.65;
    } else if (splashTime_ > 2.5) {
        opacity = std::clamp((3.2 - splashTime_) / 0.7, 0.0, 1.0);
    }

    p.save();
    p.setOpacity(opacity);

    if (!crazyLabLogo_.isNull()) {
        const QSize targetSize(390, 390);
        const QPixmap scaled = crazyLabLogo_.scaled(
            targetSize,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);
        const QPoint topLeft(
            (W - scaled.width()) / 2,
            55);
        p.drawPixmap(topLeft, scaled);
    }

    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setPen(QColor(241, 132, 28));
    p.setFont(QFont("Arial", 31, QFont::Black));
    p.drawText(QRectF(0, 450, W, 50), Qt::AlignCenter, "IZOTOPEPA");

    p.setPen(QColor(218, 220, 224));
    p.setFont(QFont("Arial", 13, QFont::DemiBold));
    p.drawText(
        QRectF(0, 498, W, 28),
        Qt::AlignCenter,
        "COMPLETE EDITION");

    p.setPen(QColor(205, 208, 212, 220));
    p.setFont(QFont("Arial", 10));
    p.drawText(
        QRectF(0, 570, W, 25),
        Qt::AlignCenter,
        "PRESS ANY KEY TO CONTINUE");
    p.restore();
}

void GameWidget::finishSplash()
{
    if (mode_ != Mode::Splash) {
        return;
    }

    mode_ = Mode::Menu;
    menuIndex_ = 0;
    splashTime_ = 3.2;
    updateWindowTitle();
    update();
}

void GameWidget::drawBackground(QPainter &p)
{
    QLinearGradient sky(0, 0, 0, H);
    sky.setColorAt(0.0, QColor(75, 155, 220));
    sky.setColorAt(0.58, QColor(145, 210, 240));
    sky.setColorAt(1.0, QColor(225, 235, 205));
    p.fillRect(rect(), sky);

    const QPointF sun(790, 95);
    QRadialGradient sunGlow(sun, 105);
    sunGlow.setColorAt(0.0, QColor(255, 245, 175, 210));
    sunGlow.setColorAt(0.35, QColor(255, 235, 140, 95));
    sunGlow.setColorAt(1.0, QColor(255, 235, 140, 0));
    p.setPen(Qt::NoPen);
    p.setBrush(sunGlow);
    p.drawEllipse(sun, 105, 105);

    p.setBrush(QColor(105, 145, 155, 155));
    const double farOffset = std::fmod(camera_.x() * 0.08, 440.0);
    for (int i = -2; i < 6; ++i) {
        const double x = i * 440.0 - farOffset;
        QPolygonF mountain;
        mountain << QPointF(x, 520)
                 << QPointF(x + 145, 285)
                 << QPointF(x + 285, 520);
        p.drawPolygon(mountain);
    }

    p.setBrush(QColor(70, 142, 102));
    const double hillOffset = std::fmod(camera_.x() * 0.18, 520.0);
    for (int i = -1; i < 9; ++i) {
        const double x = i * 520.0 - hillOffset;
        p.drawEllipse(QPointF(x + 190, 580), 235, 220);
    }

    const double cloudTravel = std::fmod(sceneTime_ * 12.0, 600.0);
    p.setBrush(QColor(255, 255, 255, 215));
    for (int i = -1; i < 9; ++i) {
        double x = i * 600.0
            - std::fmod(camera_.x() * 0.10, 600.0)
            + cloudTravel;
        const double y = 72.0 + (i % 3) * 68.0
            + qSin(sceneTime_ * 0.45 + i) * 4.0;
        const double scale = 0.82 + (i % 3) * 0.12;

        p.drawEllipse(QPointF(x + 35 * scale, y), 30 * scale, 25 * scale);
        p.drawEllipse(QPointF(x + 70 * scale, y - 10 * scale), 38 * scale, 34 * scale);
        p.drawEllipse(QPointF(x + 110 * scale, y), 30 * scale, 25 * scale);
    }
}

void GameWidget::drawPlatforms(QPainter&p){/* Static geometry is intentionally drawn from the level-independent collision silhouette via world objects; ground strip provides visual continuity. */p.fillRect(QRectF(0,570,W,70),QColor(70,110,50));p.fillRect(QRectF(0,570,W,8),QColor(120,175,75));}
void GameWidget::drawHud(QPainter &p)
{
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(20, 20, 28, 220));
    p.drawRoundedRect(QRectF(12, 12, 430, 80), 8, 8);
    p.fillRect(QRectF(25, 28, 170, 20), QColor(60, 60, 70));
    p.fillRect(
        QRectF(
            25,
            28,
            170.0 * std::max(0, world_.player().health()) / 5.0,
            20),
        QColor(210, 55, 55));

    p.setPen(Qt::white);
    p.setFont(QFont("Arial", 11, QFont::Bold));
    p.drawText(
        QRectF(28, 25, 180, 25),
        Qt::AlignVCenter,
        QString("Health %1/5")
            .arg(std::max(0, world_.player().health())));
    p.drawText(
        QRectF(25, 55, 400, 25),
        Qt::AlignVCenter,
        QString("Ammo %1     Score %2     %3")
            .arg(world_.player().ammo())
            .arg(world_.player().score())
            .arg(world_.levelName()));

    if (!world_.message().isEmpty()) {
        p.setBrush(QColor(0, 0, 0, 170));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(QRectF(330, 105, 300, 42), 8, 8);
        p.setPen(Qt::yellow);
        p.drawText(
            QRectF(330, 105, 300, 42),
            Qt::AlignCenter,
            world_.message());
    }

    if (developerStatusTime_ > 0.0
        && !developerStatus_.isEmpty()) {
        p.setBrush(QColor(0, 0, 0, 205));
        p.setPen(Qt::NoPen);
        p.drawRoundedRect(
            QRectF(W / 2.0 - 190, H - 66, 380, 40),
            7,
            7);
        p.setPen(QColor(120, 255, 140));
        p.drawText(
            QRectF(W / 2.0 - 180, H - 64, 360, 36),
            Qt::AlignCenter,
            developerStatus_);
    }

    if (debugOverlay_) {
        drawDebugOverlay(p);
    }

    if (developerOptions_.developerMode()) {
        const QRectF badge(W - 230, H - 66, 218, 52);
        p.setPen(QPen(QColor(255, 210, 70, 210), 1));
        p.setBrush(QColor(25, 18, 5, 205));
        p.drawRoundedRect(badge, 7, 7);
        p.setFont(QFont("Arial", 10, QFont::Bold));
        p.setPen(QColor(255, 220, 85));
        p.drawText(
            badge.adjusted(10, 5, -10, -25),
            Qt::AlignCenter,
            "DEVELOPER MODE");
        p.setFont(QFont("Arial", 9));
        p.setPen(developerOptions_.godMode
            ? QColor(255, 220, 85)
            : QColor(205, 210, 215));
        p.drawText(
            badge.adjusted(10, 25, -10, -4),
            Qt::AlignCenter,
            QString("God Mode: %1")
                .arg(developerOptions_.godMode ? "ON" : "OFF"));
    }
}
void GameWidget::drawDebugOverlay(QPainter &p)
{
    const WorldDebugStats worldStats = world_.debugStats();
    const Player &player = world_.player();

    DeveloperOverlayData data;
    data.fps = fps_;
    data.frameTimeMs = fps_ > 0.0 ? 1000.0 / fps_ : 0.0;
    data.levelNumber = directLevelFile_.isEmpty()
        ? world_.levelIndex() + 1
        : 0;
    data.levelName = world_.levelName();
    data.levelSource = directLevelFile_.isEmpty()
        ? "Campaign"
        : QFileInfo(directLevelFile_).absoluteFilePath();
    data.developerMode = developerOptions_.developerMode();
    data.progressSaving = !developerOptions_.developerMode();
    data.godMode = world_.godMode();

    data.playerPosition = player.position();
    data.playerVelocity = player.velocity();
    data.grounded = player.grounded();
    data.climbing = player.climbing();
    data.swimming = player.inWater();
    data.onIce = worldStats.playerOnIce;
    data.invulnerable = player.invulnerable();
    data.health = player.health();
    data.ammo = player.ammo();
    data.score = player.score();
    data.keys = player.keyCount();

    data.enemies.legacy = worldStats.legacyEnemies;
    data.enemies.drones = worldStats.drones;
    data.enemies.turrets = worldStats.turrets;
    data.enemies.chargers = worldStats.chargers;
    data.enemies.shields = worldStats.shields;
    data.projectiles = worldStats.projectiles;
    data.particles = worldStats.particles;

    data.cameraX = camera_.x();
    data.cameraLookAhead = camera_.lookAhead();
    data.cameraShake = worldStats.cameraShake;
    data.hitStop = worldStats.hitStop;
    data.selectedEnemy = worldStats.selectedEnemy;

    developerOverlayRenderer_.draw(
        p,
        data,
        W,
        H,
        compactDebugOverlay_);
}
void GameWidget::drawMenu(
    QPainter &p,
    const QString &title,
    const QStringList &items)
{
    // Original menu layout with a green/black palette matching the game world.
    QLinearGradient background(0, 0, 0, H);
    background.setColorAt(0.0, QColor(3, 18, 10));
    background.setColorAt(0.58, QColor(5, 31, 16));
    background.setColorAt(1.0, QColor(2, 9, 5));
    p.fillRect(rect(), background);

    // Soft green atmosphere behind the Crazy Lab watermark.
    QRadialGradient atmosphere(QPointF(W * 0.77, H * 0.46), W * 0.48);
    atmosphere.setColorAt(0.0, QColor(34, 116, 45, 72));
    atmosphere.setColorAt(0.52, QColor(15, 65, 28, 34));
    atmosphere.setColorAt(1.0, QColor(0, 0, 0, 0));
    p.fillRect(rect(), atmosphere);

    // Distant factory silhouette.
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(1, 9, 5, 235));
    for (int x = 0; x < W; x += 58) {
        const int height = 58 + ((x / 58) % 5) * 21;
        p.drawRect(QRectF(x, H - 95 - height, 42, height));
        if ((x / 58) % 3 == 0) {
            p.drawRect(QRectF(x + 12, H - 145 - height, 8, 55));
        }
    }

    // Floor and green industrial edge lighting.
    p.fillRect(QRectF(0, H - 88, W, 88), QColor(2, 8, 4));
    p.setPen(QPen(QColor(111, 238, 73, 175), 2));
    p.drawLine(QPointF(0, H - 88), QPointF(W, H - 88));
    for (int x = 32; x < W; x += 145) {
        p.drawLine(QPointF(x, H - 87), QPointF(x + 34, H - 87));
    }

    // Large faded logo watermark.
    if (!crazyLabLogo_.isNull()) {
        p.save();
        p.setOpacity(0.11);
        const QPixmap watermark = crazyLabLogo_.scaled(
            QSize(570, 570),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);
        p.drawPixmap(
            QPoint(W - watermark.width() + 80,
                   (H - watermark.height()) / 2 + 35),
            watermark);
        p.restore();
    }

    p.setRenderHint(QPainter::TextAntialiasing, true);
    p.setPen(QColor(126, 240, 62));
    p.setFont(QFont("Arial", 34, QFont::Black));
    p.drawText(QRectF(72, 62, 540, 52), Qt::AlignLeft | Qt::AlignVCenter, title);

    p.setPen(QColor(224, 236, 225));
    p.setFont(QFont("Arial", 12, QFont::DemiBold));
    p.drawText(
        QRectF(76, 109, 440, 28),
        Qt::AlignLeft | Qt::AlignVCenter,
        "COMPLETE EDITION");

    const int count = std::max(1, static_cast<int>(items.size()));
    menuIndex_ = std::clamp(menuIndex_, 0, count - 1);

    p.setFont(QFont("Arial", 16, QFont::DemiBold));
    for (int i = 0; i < items.size(); ++i) {
        const QRectF itemRect(92, 180 + i * 57, 410, 43);
        const bool selected = i == menuIndex_;

        if (selected) {
            QLinearGradient selection(itemRect.topLeft(), itemRect.topRight());
            selection.setColorAt(0.0, QColor(18, 92, 27, 235));
            selection.setColorAt(1.0, QColor(5, 31, 14, 190));
            p.setPen(QPen(QColor(128, 255, 70), 2));
            p.setBrush(selection);
            p.drawRect(itemRect);

            QPolygonF arrow;
            arrow << QPointF(itemRect.left() - 20, itemRect.center().y() - 8)
                  << QPointF(itemRect.left() - 6, itemRect.center().y())
                  << QPointF(itemRect.left() - 20, itemRect.center().y() + 8);
            p.setPen(Qt::NoPen);
            p.setBrush(QColor(151, 255, 76));
            p.drawPolygon(arrow);
            p.setPen(QColor(241, 255, 235));
        } else {
            p.setPen(QColor(213, 225, 215));
            p.setBrush(Qt::NoBrush);
        }

        p.drawText(
            itemRect.adjusted(25, 0, -10, 0),
            Qt::AlignLeft | Qt::AlignVCenter,
            items[i]);
    }

    p.setPen(QColor(129, 176, 133));
    p.setFont(QFont("Arial", 9));
    p.drawText(
        QRectF(22, H - 34, W - 44, 20),
        Qt::AlignRight | Qt::AlignVCenter,
        "© 2026 CRAZY LAB");
}

void GameWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat()) {
        return;
    }

    if (mode_ == Mode::Splash) {
        finishSplash();
        return;
    }

    const int key = event->key();
    const Qt::KeyboardModifiers modifiers = event->modifiers();
    const bool control = modifiers.testFlag(Qt::ControlModifier);

    if (key == Qt::Key_F3) {
        if (modifiers.testFlag(Qt::ShiftModifier)) {
            debugOverlay_ = true;
            compactDebugOverlay_ = !compactDebugOverlay_;
        } else {
            debugOverlay_ = !debugOverlay_;
        }
        update();
        return;
    }

    if (key == Qt::Key_F5) {
        reloadCurrentLevel();
        return;
    }

    if (key == Qt::Key_F6) {
        reloadExternalLevel();
        return;
    }

    if (control && key == Qt::Key_R) {
        restartCurrentLevel();
        return;
    }

    if (control && key == Qt::Key_PageUp) {
        switchCampaignLevel(-1);
        return;
    }

    if (control && key == Qt::Key_PageDown) {
        switchCampaignLevel(1);
        return;
    }

    if (mode_ == Mode::Playing) {
        if (key == Qt::Key_Left || key == Qt::Key_A) {
            left_ = true;
        } else if (key == Qt::Key_Right || key == Qt::Key_D) {
            right_ = true;
        } else if (key == Qt::Key_Up || key == Qt::Key_W) {
            up_ = true;
        } else if (key == Qt::Key_Down || key == Qt::Key_S) {
            down_ = true;
        } else if (key == Qt::Key_Space) {
            world_.jump();
        } else if (key == Qt::Key_F || key == Qt::Key_Control) {
            world_.shoot();
        } else if (key == Qt::Key_E) {
            world_.interact();
        } else if (key == Qt::Key_P || key == Qt::Key_Escape) {
            mode_ = Mode::Paused;
            menuIndex_ = 0;
        }
        return;
    }

    if (key == Qt::Key_Up) {
        menuIndex_ = std::max(0, menuIndex_ - 1);
        return;
    }

    if (key == Qt::Key_Down) {
        int itemCount = 5;
        if (mode_ == Mode::Paused || mode_ == Mode::GameOver) {
            itemCount = 3;
        } else if (mode_ == Mode::Settings) {
            itemCount = 3;
        } else if (mode_ == Mode::Complete) {
            itemCount = 2;
        } else if (mode_ == Mode::Help) {
            itemCount = 7;
        }
        menuIndex_ = std::min(itemCount - 1, menuIndex_ + 1);
        return;
    }

    if (key == Qt::Key_Backspace || key == Qt::Key_Escape) {
        mode_ = Mode::Menu;
        menuIndex_ = 0;
        return;
    }

    if (key != Qt::Key_Return
        && key != Qt::Key_Enter
        && key != Qt::Key_Space) {
        return;
    }

    if (mode_ == Mode::Menu) {
        if (menuIndex_ == 0) {
            startLevel(0);
        } else if (menuIndex_ == 1) {
            startLevel(unlocked_);
        } else if (menuIndex_ == 2) {
            mode_ = Mode::Help;
            menuIndex_ = 0;
        } else if (menuIndex_ == 3) {
            mode_ = Mode::Settings;
            menuIndex_ = 0;
        } else {
            close();
        }
    } else if (mode_ == Mode::Paused) {
        if (menuIndex_ == 0) {
            mode_ = Mode::Playing;
        } else if (menuIndex_ == 1) {
            startLevel(world_.levelIndex());
        } else {
            mode_ = Mode::Menu;
            menuIndex_ = 0;
        }
    } else if (mode_ == Mode::Settings) {
        if (menuIndex_ == 0) {
            world_.toggleSound();
        } else if (menuIndex_ == 1) {
            QSettings("OpenAI", "Izotopepa").clear();
            unlocked_ = 0;
        } else {
            mode_ = Mode::Menu;
            menuIndex_ = 0;
        }
    } else if (mode_ == Mode::GameOver) {
        if (menuIndex_ == 0) {
            world_.resetFromCheckpoint();
            mode_ = Mode::Playing;
        } else if (menuIndex_ == 1) {
            startLevel(world_.levelIndex());
        } else {
            mode_ = Mode::Menu;
            menuIndex_ = 0;
        }
    } else if (mode_ == Mode::Complete) {
        if (menuIndex_ == 0) {
            startLevel(world_.levelIndex() < 9
                ? world_.levelIndex() + 1
                : 0);
        } else {
            mode_ = Mode::Menu;
            menuIndex_ = 0;
        }
    }
}
void GameWidget::mousePressEvent(QMouseEvent *event)
{
    if (mode_ == Mode::Splash) {
        finishSplash();
        event->accept();
        return;
    }

    QWidget::mousePressEvent(event);
}

void GameWidget::keyReleaseEvent(QKeyEvent*e){if(e->isAutoRepeat())return;int k=e->key();if(k==Qt::Key_Left||k==Qt::Key_A)left_=false;else if(k==Qt::Key_Right||k==Qt::Key_D)right_=false;else if(k==Qt::Key_Up||k==Qt::Key_W)up_=false;else if(k==Qt::Key_Down||k==Qt::Key_S)down_=false;else if(k==Qt::Key_Space)world_.stopJump();}
void GameWidget::saveSettings(){}
