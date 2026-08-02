#include "onewayplatform.h"

OneWayPlatform::OneWayPlatform(const QRectF &rect)
    : rect_(rect)
{
}

const QRectF &OneWayPlatform::rect() const
{
    return rect_;
}

void OneWayPlatform::draw(QPainter &painter, double cameraX) const
{
    const QRectF visible = rect_.translated(-cameraX, 0);

    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(66, 82, 96));
    painter.drawRoundedRect(visible, 5, 5);

    painter.setBrush(QColor(135, 205, 225));
    painter.drawRoundedRect(
        QRectF(visible.left(), visible.top(), visible.width(), 6),
        3,
        3);

    painter.setPen(QPen(QColor(205, 240, 250), 2));
    const double spacing = 22.0;
    for (double x = visible.left() + 10.0;
         x < visible.right() - 5.0;
         x += spacing) {
        painter.drawLine(
            QPointF(x, visible.center().y() + 4.0),
            QPointF(x + 7.0, visible.center().y() - 3.0));
        painter.drawLine(
            QPointF(x + 7.0, visible.center().y() - 3.0),
            QPointF(x + 14.0, visible.center().y() + 4.0));
    }
    painter.restore();
}
