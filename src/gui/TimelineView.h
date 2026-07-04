#pragma once

#include <QGraphicsView>

namespace notwork::gui {

class TimelineView : public QGraphicsView {
    Q_OBJECT
public:
    explicit TimelineView(QGraphicsScene* scene, QWidget* parent = nullptr);
};

} // namespace notwork::gui
