#include "developeroverlay.h"

#include <QFont>
#include <QFontMetrics>
#include <QPainter>

namespace {

QString yesNo(bool value)
{
    return value ? "yes" : "no";
}

QString playerSurface(const DeveloperOverlayData &data)
{
    if (data.swimming) {
        return "water";
    }
    if (data.climbing) {
        return "ladder";
    }
    if (data.onIce) {
        return "ice";
    }
    if (data.grounded) {
        return "ground";
    }
    return "air";
}

void drawSectionTitle(
    QPainter &painter,
    const QRectF &lineRect,
    const QString &title)
{
    painter.setPen(QColor(100, 225, 255));
    painter.drawText(lineRect, Qt::AlignLeft | Qt::AlignVCenter, title);
}

void drawValueLine(
    QPainter &painter,
    const QRectF &lineRect,
    const QString &label,
    const QString &value,
    const QColor &valueColor = Qt::white)
{
    const double labelWidth = 118.0;

    painter.setPen(QColor(190, 200, 210));
    painter.drawText(
        QRectF(lineRect.left(), lineRect.top(), labelWidth, lineRect.height()),
        Qt::AlignLeft | Qt::AlignVCenter,
        label);

    painter.setPen(valueColor);
    painter.drawText(
        QRectF(
            lineRect.left() + labelWidth,
            lineRect.top(),
            lineRect.width() - labelWidth,
            lineRect.height()),
        Qt::AlignLeft | Qt::AlignVCenter,
        value);
}

} // namespace

void DeveloperOverlay::draw(
    QPainter &painter,
    const DeveloperOverlayData &data,
    int viewportWidth,
    int viewportHeight,
    bool compact) const
{
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setFont(QFont("Monospace", compact ? 9 : 10));

    if (compact) {
        const QString text = QString(
            "FPS %1  L%2  Enemies %3  Pj %4  Fx %5  %6")
            .arg(data.fps, 0, 'f', 0)
            .arg(data.levelNumber > 0 ? QString::number(data.levelNumber) : "file")
            .arg(data.enemies.total())
            .arg(data.projectiles)
            .arg(data.particles)
            .arg(data.godMode ? "GOD" : playerSurface(data).toUpper());

        const QFontMetrics metrics(painter.font());
        const int width = metrics.horizontalAdvance(text) + 24;
        const QRectF panel(
            viewportWidth - width - 12,
            12,
            width,
            30);

        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0, 0, 0, 205));
        painter.drawRoundedRect(panel, 6, 6);
        painter.setPen(data.godMode ? QColor(255, 220, 70)
                                    : QColor(120, 255, 140));
        painter.drawText(panel.adjusted(12, 0, -12, 0), Qt::AlignVCenter, text);
        painter.restore();
        return;
    }

    const double panelWidth = 370.0;
    const double panelHeight = qMin(610.0, viewportHeight - 24.0);
    const QRectF panel(
        viewportWidth - panelWidth - 12.0,
        12.0,
        panelWidth,
        panelHeight);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(5, 8, 12, 218));
    painter.drawRoundedRect(panel, 8, 8);

    const QRectF content = panel.adjusted(14, 10, -14, -10);
    const double lineHeight = 18.0;
    double y = content.top();

    painter.setPen(QColor(120, 255, 140));
    painter.setFont(QFont("Monospace", 11, QFont::Bold));
    painter.drawText(
        QRectF(content.left(), y, content.width(), 22),
        Qt::AlignLeft | Qt::AlignVCenter,
        "IZOTOPEPA DEVELOPER HUD");
    y += 25.0;

    painter.setFont(QFont("Monospace", 9));

    auto section = [&](const QString &title) {
        drawSectionTitle(
            painter,
            QRectF(content.left(), y, content.width(), lineHeight),
            title);
        y += lineHeight;
    };

    auto value = [&](const QString &label,
                     const QString &text,
                     const QColor &color = Qt::white) {
        drawValueLine(
            painter,
            QRectF(content.left(), y, content.width(), lineHeight),
            label,
            text,
            color);
        y += lineHeight;
    };

    section("SESSION");
    value(
        "Level",
        data.levelNumber > 0
            ? QString("%1  %2").arg(data.levelNumber).arg(data.levelName)
            : data.levelName);
    value("Source", data.levelSource);
    value(
        "Developer",
        yesNo(data.developerMode),
        data.developerMode ? QColor(255, 220, 70) : Qt::white);
    value(
        "God mode",
        yesNo(data.godMode),
        data.godMode ? QColor(255, 220, 70) : Qt::white);

    section("PERFORMANCE");
    value("FPS", QString::number(data.fps, 'f', 1),
          data.fps < 45.0 ? QColor(255, 100, 90) : Qt::white);
    value("Frame", QString("%1 ms").arg(data.frameTimeMs, 0, 'f', 2));

    section("PLAYER");
    value(
        "Position",
        QString("%1, %2")
            .arg(data.playerPosition.x(), 0, 'f', 1)
            .arg(data.playerPosition.y(), 0, 'f', 1));
    value(
        "Velocity",
        QString("%1, %2")
            .arg(data.playerVelocity.x(), 0, 'f', 1)
            .arg(data.playerVelocity.y(), 0, 'f', 1));
    value("Surface", playerSurface(data));
    value("Grounded", yesNo(data.grounded));
    value("Climbing", yesNo(data.climbing));
    value("Swimming", yesNo(data.swimming));
    value("Invulnerable", yesNo(data.invulnerable));
    value(
        "Stats",
        QString("HP %1  Ammo %2  Score %3")
            .arg(data.health)
            .arg(data.ammo)
            .arg(data.score));
    value("Keys", QString::number(data.keys));

    section("ENEMIES");
    value(
        "Alive total",
        QString::number(data.enemies.total()));
    value(
        "L / D / T",
        QString("%1 / %2 / %3")
            .arg(data.enemies.legacy)
            .arg(data.enemies.drones)
            .arg(data.enemies.turrets));
    value(
        "C / S",
        QString("%1 / %2")
            .arg(data.enemies.chargers)
            .arg(data.enemies.shields));

    section("COMBAT / CAMERA");
    value(
        "Projectiles",
        QString::number(data.projectiles));
    value("Particles", QString::number(data.particles));
    value(
        "Camera",
        QString("x %1  look %2")
            .arg(data.cameraX, 0, 'f', 1)
            .arg(data.cameraLookAhead, 0, 'f', 1));
    value("Shake", QString::number(data.cameraShake, 'f', 2));
    value(
        "Hit stop",
        data.hitStop ? "ON" : "off",
        data.hitStop ? QColor(255, 220, 70) : Qt::white);

    section("SELECTED ENEMY");
    painter.setPen(QColor(225, 230, 235));
    painter.drawText(
        QRectF(
            content.left(),
            y,
            content.width(),
            qMax(38.0, content.bottom() - y - 18.0)),
        Qt::AlignLeft | Qt::AlignTop | Qt::TextWordWrap,
        data.selectedEnemy);

    painter.setPen(QColor(150, 160, 170));
    painter.drawText(
        QRectF(
            content.left(),
            content.bottom() - 18.0,
            content.width(),
            18.0),
        Qt::AlignRight | Qt::AlignVCenter,
        "F3 hide   Shift+F3 compact");

    painter.restore();
}
