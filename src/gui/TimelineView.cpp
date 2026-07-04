#include "gui/TimelineView.h"

namespace notwork::gui {

TimelineView::TimelineView(QGraphicsScene* scene, QWidget* parent)
    : QGraphicsView(scene, parent) {
    setFrameShape(QFrame::NoFrame);
    // Do NOT set backgroundBrush on the view — it would short-circuit
    // QGraphicsScene::drawBackground() where we paint the ruler and lanes.
    setAlignment(Qt::AlignTop | Qt::AlignLeft);
    setRenderHint(QPainter::TextAntialiasing, true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setDragMode(QGraphicsView::NoDrag);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setTransformationAnchor(QGraphicsView::NoAnchor);
}

} // namespace notwork::gui
