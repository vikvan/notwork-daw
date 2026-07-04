#include "gui/ClipItem.h"

#include "gui/TimelineScene.h"
#include "model/Project.h"

#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include <algorithm>
#include <cmath>

namespace notwork::gui {

ClipItem::ClipItem(TimelineScene* scene,
                   notwork::model::Project* project,
                   int trackIndex,
                   int eventIndex,
                   qreal width,
                   qreal height,
                   const QColor& color,
                   QString name)
    : scene_(scene),
      project_(project),
      trackIndex_(trackIndex),
      eventIndex_(eventIndex),
      width_(width),
      height_(height),
      color_(color),
      name_(std::move(name)) {
    setFlag(ItemIsSelectable, true);
    setFlag(ItemIsMovable, true);
    setFlag(ItemSendsGeometryChanges, true);
    setCursor(Qt::OpenHandCursor);
}

void ClipItem::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) {
    const QRectF r(0, 0, width_, height_);
    p->setRenderHint(QPainter::Antialiasing, true);

    QColor fill = color_;
    fill.setAlpha(210);
    p->setBrush(fill);
    p->setPen(QPen(isSelected() ? QColor(255, 255, 255) : color_.lighter(130),
                   isSelected() ? 2 : 1));
    p->drawRoundedRect(r.adjusted(0.5, 0.5, -0.5, -0.5), 4, 4);

    QColor header = color_.darker(140);
    header.setAlpha(230);
    p->setBrush(header);
    p->setPen(Qt::NoPen);
    p->drawRoundedRect(QRectF(0, 0, width_, 16), 4, 4);
    p->fillRect(QRectF(0, 12, width_, 4), header);

    p->setPen(QColor(240, 240, 240));
    p->drawText(QRectF(6, 0, width_ - 12, 16), Qt::AlignVCenter | Qt::AlignLeft, name_);
}

QVariant ClipItem::itemChange(GraphicsItemChange change, const QVariant& value) {
    if (change == ItemPositionChange && scene() == scene_) {
        QPointF np = value.toPointF();
        np.setX(std::max<qreal>(0.0, np.x()));

        const int trackCount = static_cast<int>(project_->tracks().size());
        if (trackCount > 0) {
            const qreal rowH   = TimelineScene::kTrackRowHeight;
            const qreal top    = TimelineScene::kRulerHeight;
            const qreal margin = 4.0;
            const qreal rel    = np.y() - top - margin;
            int row = static_cast<int>(std::round(rel / rowH));
            row = std::clamp(row, 0, trackCount - 1);
            np.setY(top + row * rowH + margin);
        }
        return np;
    }
    return QGraphicsItem::itemChange(change, value);
}

void ClipItem::mousePressEvent(QGraphicsSceneMouseEvent* e) {
    setCursor(Qt::ClosedHandCursor);
    QGraphicsItem::mousePressEvent(e);
}

void ClipItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* e) {
    QGraphicsItem::mouseReleaseEvent(e);
    setCursor(Qt::OpenHandCursor);

    const qreal rowH   = TimelineScene::kTrackRowHeight;
    const qreal top    = TimelineScene::kRulerHeight;
    const qreal margin = 4.0;
    const int newTrack = std::clamp(static_cast<int>(std::round((pos().y() - top - margin) / rowH)),
                                    0, static_cast<int>(project_->tracks().size()) - 1);
    const int64_t newStart = static_cast<int64_t>(pos().x() * scene_->samplesPerPixel());

    project_->moveEvent(trackIndex_, eventIndex_, newTrack, newStart);
}

} // namespace notwork::gui
