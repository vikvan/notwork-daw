#include "gui/TimelineScene.h"

#include "engine/AudioClip.h"
#include "engine/AudioEngine.h"
#include "gui/ClipItem.h"
#include "model/Project.h"
#include "model/Track.h"

#include <QGraphicsLineItem>
#include <QPainter>
#include <QPen>
#include <QTimer>

namespace notwork::gui {

TimelineScene::TimelineScene(notwork::model::Project* project,
                             notwork::engine::AudioEngine* engine,
                             QObject* parent)
    : QGraphicsScene(parent), project_(project), engine_(engine) {
    const int sr        = project_->sampleRate();
    const qreal totalPx = (sr * kSceneDurationSeconds) / samplesPerPixel_;
    const qreal totalH  = kRulerHeight + kTrackRowHeight * static_cast<qreal>(project_->tracks().size());
    setSceneRect(0, 0, totalPx, totalH);

    playheadItem_ = new QGraphicsLineItem(0, 0, 0, totalH);
    QPen pen(QColor(230, 60, 60));
    pen.setWidthF(1.5);
    pen.setCosmetic(true);
    playheadItem_->setPen(pen);
    playheadItem_->setZValue(1000);
    addItem(playheadItem_);

    connect(project_, &notwork::model::Project::changed, this, [this]{
        const qreal h = kRulerHeight + kTrackRowHeight * static_cast<qreal>(project_->tracks().size());
        setSceneRect(sceneRect().x(), sceneRect().y(), sceneRect().width(), h);
        playheadItem_->setLine(0, 0, 0, h);
        rebuildClips();
    });

    playheadTimer_ = new QTimer(this);
    playheadTimer_->setInterval(33);
    connect(playheadTimer_, &QTimer::timeout, this, &TimelineScene::updatePlayhead);
    playheadTimer_->start();

    rebuildClips();
    updatePlayhead();
}

void TimelineScene::updatePlayhead() {
    if (!engine_ || !playheadItem_) return;
    const qreal x = engine_->playheadSamples() / samplesPerPixel_;
    playheadItem_->setPos(x, 0);
}

void TimelineScene::rebuildClips() {
    // Remove existing ClipItems.
    const auto items = this->items();
    for (auto* it : items) {
        if (dynamic_cast<ClipItem*>(it)) removeItem(it), delete it;
    }

    const auto& tracks = project_->tracks();
    for (size_t ti = 0; ti < tracks.size(); ++ti) {
        const auto& t = tracks[ti];
        for (size_t ei = 0; ei < t.events.size(); ++ei) {
            const auto& ev = t.events[ei];
            const qreal x = ev.startSamples  / samplesPerPixel_;
            const qreal w = ev.lengthSamples / samplesPerPixel_;
            const qreal y = rowY(static_cast<int>(ti)) + 4;
            const qreal h = kTrackRowHeight - 8;
            auto audio = ev.filePath.isEmpty()
                             ? std::shared_ptr<const notwork::engine::AudioClip>()
                             : notwork::engine::AudioClip::loadCached(ev.filePath);

            auto* clip = new ClipItem(this, project_,
                                      static_cast<int>(ti),
                                      static_cast<int>(ei),
                                      w, h, t.color, ev.name,
                                      audio, ev.offsetSamples);
            clip->setPos(x, y);
            addItem(clip);
        }
    }
}

void TimelineScene::drawBackground(QPainter* p, const QRectF& rect) {
    // Base fill.
    p->fillRect(rect, QColor(30, 31, 34));

    // Track lanes (alternating).
    const auto& tracks = project_->tracks();
    for (size_t i = 0; i < tracks.size(); ++i) {
        const qreal y = rowY(static_cast<int>(i));
        QRectF laneRect(rect.left(), y, rect.width(), kTrackRowHeight);
        p->fillRect(laneRect, (i % 2 == 0) ? QColor(38, 39, 43) : QColor(35, 36, 40));
        // Bottom separator.
        p->fillRect(QRectF(rect.left(), y + kTrackRowHeight - 1, rect.width(), 1),
                    QColor(26, 27, 30));
    }

    // Ruler background — darker band with accent underline.
    QRectF rulerRect(rect.left(), 0, rect.width(), kRulerHeight);
    p->fillRect(rulerRect, QColor(18, 20, 24));
    p->fillRect(QRectF(rect.left(), kRulerHeight - 2, rect.width(), 2),
                QColor(58, 123, 213));

    // Time ticks: one per second, labels every 5 seconds.
    const int sr = project_->sampleRate();
    const qreal pxPerSecond = static_cast<qreal>(sr) / samplesPerPixel_;
    if (pxPerSecond <= 0) return;

    const int firstSec = static_cast<int>(std::max(0.0, rect.left() / pxPerSecond));
    const int lastSec  = static_cast<int>((rect.right() / pxPerSecond) + 1);

    for (int s = firstSec; s <= lastSec; ++s) {
        const qreal x = s * pxPerSecond;
        const bool major = (s % 5 == 0);
        p->setPen(major ? QColor(210, 214, 222) : QColor(120, 124, 132));
        p->drawLine(QPointF(x, major ? 4 : 18), QPointF(x, kRulerHeight - 2));
        if (major) {
            // Faint gridline down through the lanes.
            p->setPen(QColor(255, 255, 255, 12));
            p->drawLine(QPointF(x, kRulerHeight), QPointF(x, rect.bottom()));
        }
    }

    p->setPen(QColor(230, 232, 238));
    QFont f = p->font();
    f.setPointSizeF(std::max(9.0, f.pointSizeF()));
    f.setBold(true);
    p->setFont(f);
    for (int s = firstSec; s <= lastSec; ++s) {
        if (s % 5 != 0) continue;
        const qreal x = s * pxPerSecond;
        p->drawText(QPointF(x + 4, 18), QString::number(s) + "s");
    }
}

} // namespace notwork::gui
