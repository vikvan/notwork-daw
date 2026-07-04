#pragma once

#include <QGraphicsItem>
#include <QColor>
#include <QString>

namespace notwork::model { class Project; }

namespace notwork::gui {

class TimelineScene;

class ClipItem : public QGraphicsItem {
public:
    ClipItem(TimelineScene* scene,
             notwork::model::Project* project,
             int trackIndex,
             int eventIndex,
             qreal width,
             qreal height,
             const QColor& color,
             QString name);

    QRectF boundingRect() const override { return QRectF(0, 0, width_, height_); }
    void   paint(QPainter* p, const QStyleOptionGraphicsItem* opt, QWidget* w) override;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void     mousePressEvent   (QGraphicsSceneMouseEvent* e) override;
    void     mouseReleaseEvent (QGraphicsSceneMouseEvent* e) override;

private:
    TimelineScene*           scene_;
    notwork::model::Project* project_;
    int                      trackIndex_;
    int                      eventIndex_;

    qreal   width_;
    qreal   height_;
    QColor  color_;
    QString name_;
};

} // namespace notwork::gui
