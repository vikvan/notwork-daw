#include "gui/ClipItem.h"

#include <QPainter>

namespace notwork::gui {

ClipItem::ClipItem(qreal width, qreal height, const QColor& color, QString name)
    : width_(width), height_(height), color_(color), name_(std::move(name)) {
    setFlag(ItemIsSelectable, true);
    // Dragging enabled in a later step; for now items are static.
}

void ClipItem::paint(QPainter* p, const QStyleOptionGraphicsItem*, QWidget*) {
    const QRectF r(0, 0, width_, height_);
    p->setRenderHint(QPainter::Antialiasing, true);

    QColor fill = color_;
    fill.setAlpha(210);
    p->setBrush(fill);
    p->setPen(QPen(color_.lighter(130), 1));
    p->drawRoundedRect(r.adjusted(0.5, 0.5, -0.5, -0.5), 4, 4);

    // Header strip with name.
    QColor header = color_.darker(140);
    header.setAlpha(230);
    p->setBrush(header);
    p->setPen(Qt::NoPen);
    p->drawRoundedRect(QRectF(0, 0, width_, 16), 4, 4);
    p->fillRect(QRectF(0, 12, width_, 4), header);

    p->setPen(QColor(240, 240, 240));
    p->drawText(QRectF(6, 0, width_ - 12, 16), Qt::AlignVCenter | Qt::AlignLeft, name_);
}

} // namespace notwork::gui
