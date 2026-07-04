#pragma once

#include <QGraphicsItem>
#include <QColor>
#include <QString>

namespace notwork::gui {

class ClipItem : public QGraphicsItem {
public:
    ClipItem(qreal width, qreal height, const QColor& color, QString name);

    QRectF boundingRect() const override { return QRectF(0, 0, width_, height_); }
    void   paint(QPainter* p, const QStyleOptionGraphicsItem* opt, QWidget* w) override;

private:
    qreal   width_;
    qreal   height_;
    QColor  color_;
    QString name_;
};

} // namespace notwork::gui
