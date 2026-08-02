#include "levelcanvas.h"
#include <QPainter>
#include <QWheelEvent>
#include <algorithm>
#include <cmath>

LevelCanvas::LevelCanvas(QWidget *parent)
    : QWidget(parent)
{
    setMinimumSize(960, 640);
}

void LevelCanvas::setDocument(const LevelDocument *document)
{
    document_ = document;
    updateGeometry();
    update();
}

void LevelCanvas::setZoom(double zoom)
{
    zoom_ = std::clamp(zoom, 0.25, 4.0);
    updateGeometry();
    update();
}

QSize LevelCanvas::sizeHint() const
{
    if (!document_)
        return QSize(960, 640);

    return QSize(
        std::ceil(document_->worldSize().width() * zoom_),
        std::ceil(document_->worldSize().height() * zoom_));
}

QRectF LevelCanvas::scaled(const QRectF &rect) const
{
    return QRectF(
        rect.x() * zoom_, rect.y() * zoom_,
        rect.width() * zoom_, rect.height() * zoom_);
}

QPointF LevelCanvas::scaled(const QPointF &point) const
{
    return point * zoom_;
}

void LevelCanvas::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        setZoom(zoom_ * (event->angleDelta().y() > 0 ? 1.15 : 1.0 / 1.15));
        adjustSize();
        event->accept();
        return;
    }

    QWidget::wheelEvent(event);
}

void LevelCanvas::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor(38, 43, 50));

    if (!document_) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, "Open a JSON level");
        return;
    }

    const QRectF world(
        0, 0,
        document_->worldSize().width() * zoom_,
        document_->worldSize().height() * zoom_);

    painter.fillRect(world, QColor(125, 190, 225));
    drawGrid(painter);
    drawObjects(painter);

    painter.setPen(QPen(Qt::black, 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(world);
}

void LevelCanvas::drawGrid(QPainter &painter)
{
    const double step = 32.0 * zoom_;
    painter.setPen(QPen(QColor(255, 255, 255, 35), 1));

    for (double x = 0; x < width(); x += step)
        painter.drawLine(QPointF(x, 0), QPointF(x, height()));

    for (double y = 0; y < height(); y += step)
        painter.drawLine(QPointF(0, y), QPointF(width(), y));
}

void LevelCanvas::drawObjects(QPainter &painter)
{
    for (const QRectF &platform : document_->platforms()) {
        const QRectF r = scaled(platform);
        painter.fillRect(r, QColor(91, 71, 48));
        painter.fillRect(
            QRectF(r.x(), r.y(), r.width(), 7 * zoom_),
            QColor(105, 175, 80));
    }

    for (const EditorMovingPlatform &platform :
         document_->movingPlatforms()) {
        const QRectF r = scaled(platform.rect);
        painter.fillRect(r, QColor(175, 115, 55));
        painter.setPen(QPen(QColor(255, 220, 100), 2));
        painter.drawRect(r);
        painter.setPen(QPen(QColor(255, 220, 100, 150), 2, Qt::DashLine));
        painter.drawLine(
            QPointF(platform.minX * zoom_, r.center().y()),
            QPointF(platform.maxX * zoom_, r.center().y()));
    }

    painter.setPen(QPen(QColor(165, 105, 45), 4 * zoom_));
    for (const QRectF &ladder : document_->ladders()) {
        const QRectF r = scaled(ladder);
        painter.drawLine(r.topLeft(), r.bottomLeft());
        painter.drawLine(r.topRight(), r.bottomRight());

        for (double y = r.top() + 10 * zoom_;
             y < r.bottom(); y += 16 * zoom_)
            painter.drawLine(QPointF(r.left(), y), QPointF(r.right(), y));
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(215, 60, 55));
    for (const QRectF &spike : document_->spikes()) {
        const QRectF r = scaled(spike);
        const double step = 18 * zoom_;

        for (double x = r.left(); x < r.right(); x += step) {
            QPolygonF triangle;
            triangle << QPointF(x, r.bottom())
                     << QPointF(x + step / 2, r.top())
                     << QPointF(x + step, r.bottom());
            painter.drawPolygon(triangle);
        }
    }

    painter.setBrush(QColor(250, 220, 55));
    painter.setPen(QPen(QColor(100, 75, 20), 2));
    for (const QPointF &coin : document_->coins())
        painter.drawEllipse(scaled(coin) + QPointF(10, 10) * zoom_,
                            9 * zoom_, 9 * zoom_);

    for (const EditorEnemy &enemy : document_->enemies()) {
        const QRectF r(
            enemy.position.x() * zoom_,
            enemy.position.y() * zoom_,
            46 * zoom_, 68 * zoom_);

        QColor color(180, 70, 80);
        if (enemy.kind == "shooter") color = QColor(120, 80, 190);
        if (enemy.kind == "jumper") color = QColor(220, 130, 45);
        if (enemy.kind == "boss") color = QColor(110, 30, 30);

        painter.setBrush(color);
        painter.setPen(QPen(Qt::white, 2));
        painter.drawRoundedRect(r, 7, 7);
        painter.drawText(r, Qt::AlignCenter, enemy.kind.left(1).toUpper());
    }

    for (const EditorPickup &pickup : document_->pickups()) {
        const QRectF r(
            pickup.position.x() * zoom_,
            pickup.position.y() * zoom_,
            28 * zoom_, 28 * zoom_);

        painter.setBrush(
            pickup.kind == "health"
                ? QColor(220, 55, 70)
                : QColor(55, 180, 225));
        painter.setPen(QPen(Qt::white, 2));
        painter.drawRoundedRect(r, 5, 5);
        painter.drawText(r, Qt::AlignCenter,
                         pickup.kind == "health" ? "+" : "A");
    }

    for (const EditorDoor &door : document_->doors()) {
        const QRectF r = scaled(door.rect);
        painter.fillRect(r, QColor(90, 65, 45));
        painter.setPen(QPen(QColor(240, 200, 80), 2));
        painter.drawRect(r);
        painter.drawText(r, Qt::AlignCenter, door.key);
    }

    for (const EditorNamedPoint &item : document_->switches()) {
        const QRectF r(
            item.position.x() * zoom_,
            item.position.y() * zoom_,
            28 * zoom_, 28 * zoom_);
        painter.setBrush(QColor(190, 55, 55));
        painter.setPen(QPen(Qt::white, 2));
        painter.drawRoundedRect(r, 4, 4);
        painter.drawText(r, Qt::AlignCenter, "S");
    }

    for (const EditorNamedPoint &key : document_->keys()) {
        const QPointF p = scaled(key.position);
        painter.setPen(QPen(QColor(255, 220, 40), 5));
        painter.drawLine(p, p + QPointF(26, 0) * zoom_);
        painter.drawEllipse(p, 6 * zoom_, 6 * zoom_);
    }

    if (document_->checkpoint().x() >= 0) {
        const QPointF p = scaled(document_->checkpoint());
        painter.setPen(QPen(QColor(45, 180, 225), 5));
        painter.drawLine(p, p + QPointF(0, -70) * zoom_);
    }

    const QRectF goal = scaled(document_->goal());
    painter.setPen(QPen(QColor(230, 65, 65), 5));
    painter.drawLine(goal.bottomLeft(), goal.topLeft());

    const QPointF spawn = scaled(document_->playerSpawn());
    const QRectF player(
        spawn.x(), spawn.y(), 46 * zoom_, 86 * zoom_);
    painter.setPen(QPen(Qt::white, 2));
    painter.setBrush(QColor(50, 120, 240));
    painter.drawEllipse(player);
    painter.drawText(player, Qt::AlignCenter, "P");
}
