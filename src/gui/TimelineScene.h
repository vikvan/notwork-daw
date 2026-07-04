#pragma once

#include <QGraphicsScene>

namespace notwork::model { class Project; }

namespace notwork::gui {

class TimelineScene : public QGraphicsScene {
    Q_OBJECT
public:
    static constexpr qreal kRulerHeight     = 30.0;
    static constexpr qreal kTrackRowHeight  = 72.0;
    static constexpr qreal kDefaultSamplesPerPixel = 1024.0;
    static constexpr qreal kSceneDurationSeconds  = 90.0;

    explicit TimelineScene(notwork::model::Project* project, QObject* parent = nullptr);

    qreal samplesPerPixel() const { return samplesPerPixel_; }
    qreal rowY(int trackIndex) const { return kRulerHeight + trackIndex * kTrackRowHeight; }

protected:
    void drawBackground(QPainter* p, const QRectF& rect) override;

private:
    void rebuildClips();

    notwork::model::Project* project_;
    qreal samplesPerPixel_ = kDefaultSamplesPerPixel;
};

} // namespace notwork::gui
