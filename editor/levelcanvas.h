#pragma once
#include "leveldocument.h"
#include <QWidget>

class LevelCanvas : public QWidget
{
public:
    explicit LevelCanvas(QWidget *parent = nullptr);

    void setDocument(const LevelDocument *document);
    void setZoom(double zoom);
    double zoom() const { return zoom_; }

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;
    void wheelEvent(QWheelEvent *) override;

private:
    QRectF scaled(const QRectF &rect) const;
    QPointF scaled(const QPointF &point) const;
    void drawGrid(QPainter &painter);
    void drawObjects(QPainter &painter);

    const LevelDocument *document_ = nullptr;
    double zoom_ = 1.0;
};
