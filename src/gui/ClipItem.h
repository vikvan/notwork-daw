#pragma once

#include <QGraphicsItem>
#include <QColor>
#include <QString>

#include <memory>
#include <utility>
#include <vector>

namespace notwork::engine { class AudioClip; }
namespace notwork::model  { class Project;   }

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
             QString name,
             std::shared_ptr<const notwork::engine::AudioClip> clip,
             int64_t offsetSamples);

    QRectF boundingRect() const override { return QRectF(0, 0, width_, height_); }
    void   paint(QPainter* p, const QStyleOptionGraphicsItem* opt, QWidget* w) override;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void     mousePressEvent   (QGraphicsSceneMouseEvent* e) override;
    void     mouseReleaseEvent (QGraphicsSceneMouseEvent* e) override;

private:
    void computePeaks();

    TimelineScene*           scene_;
    notwork::model::Project* project_;
    int                      trackIndex_;
    int                      eventIndex_;

    qreal   width_;
    qreal   height_;
    QColor  color_;
    QString name_;

    std::shared_ptr<const notwork::engine::AudioClip> clip_;
    int64_t offsetSamples_ = 0;
    std::vector<std::pair<float, float>> peaks_;  // min/max per pixel column
};

} // namespace notwork::gui
