#include "gui/ClipItem.h"

#include "engine/AudioClip.h"
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
                   QString name,
                   std::shared_ptr<const notwork::engine::AudioClip> clip,
                   int64_t offsetSamples)
    : scene_(scene),
      project_(project),
      trackIndex_(trackIndex),
      eventIndex_(eventIndex),
      width_(width),
      height_(height),
      color_(color),
      name_(std::move(name)),
      clip_(std::move(clip)),
      offsetSamples_(offsetSamples) {
    setFlag(ItemIsSelectable, true);
    setFlag(ItemIsMovable, true);
    setFlag(ItemSendsGeometryChanges, true);
    setCursor(Qt::OpenHandCursor);
    computePeaks();
}

void ClipItem::computePeaks() {
    if (!clip_ || !clip_->valid() || width_ <= 0) return;

    const qreal spp = scene_ ? scene_->samplesPerPixel() : 1024.0;
    const int   ch  = clip_->channels();
    const auto& d   = clip_->data();
    const int64_t total = clip_->frames();

    const int cols = static_cast<int>(std::ceil(width_));
    peaks_.assign(cols, {0.0f, 0.0f});
    for (int x = 0; x < cols; ++x) {
        const int64_t s0 = offsetSamples_ + static_cast<int64_t>(x       * spp);
        const int64_t s1 = offsetSamples_ + static_cast<int64_t>((x + 1) * spp);
        const int64_t hi = std::min<int64_t>(s1, total);
        float mn = 0.0f, mx = 0.0f;
        for (int64_t i = std::max<int64_t>(0, s0); i < hi; ++i) {
            const float s = d[static_cast<std::size_t>(i) * ch];  // channel 0
            if (s < mn) mn = s;
            if (s > mx) mx = s;
        }
        peaks_[x] = {mn, mx};
    }
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

    if (!peaks_.empty()) {
        const qreal top    = 18.0;
        const qreal bottom = height_ - 2.0;
        const qreal mid    = (top + bottom) * 0.5;
        const qreal halfH  = (bottom - top) * 0.5;

        QColor wf(255, 255, 255, 200);
        p->setPen(QPen(wf, 1.0));
        const int cols = static_cast<int>(peaks_.size());
        for (int x = 0; x < cols; ++x) {
            const auto [mn, mx] = peaks_[x];
            if (mn == 0.0f && mx == 0.0f) continue;
            const qreal yHi = mid - std::clamp<qreal>(mx, -1.0, 1.0) * halfH;
            const qreal yLo = mid - std::clamp<qreal>(mn, -1.0, 1.0) * halfH;
            p->drawLine(QPointF(x + 0.5, yHi), QPointF(x + 0.5, yLo));
        }
        // Zero-line for scale reference.
        p->setPen(QPen(QColor(255, 255, 255, 40), 1.0));
        p->drawLine(QPointF(0, mid), QPointF(width_, mid));
    }
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
