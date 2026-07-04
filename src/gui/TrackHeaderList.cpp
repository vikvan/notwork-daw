#include "gui/TrackHeaderList.h"

#include "gui/TrackHeader.h"
#include "model/Project.h"

#include <QVBoxLayout>

namespace notwork::gui {

TrackHeaderList::TrackHeaderList(notwork::model::Project* project,
                                 int inChannels,
                                 int outChannels,
                                 QWidget* parent)
    : QScrollArea(parent),
      project_(project),
      inChannels_(inChannels),
      outChannels_(outChannels) {
    setWidgetResizable(true);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setStyleSheet("QScrollArea { background: #1e1f22; }");

    content_ = new QWidget;
    content_->setStyleSheet("background: #1e1f22;");
    layout_  = new QVBoxLayout(content_);
    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->setSpacing(0);
    layout_->addStretch();
    setWidget(content_);

    connect(project_, &notwork::model::Project::changed, this, &TrackHeaderList::rebuild);
    rebuild();
}

void TrackHeaderList::setChannels(int inChannels, int outChannels) {
    inChannels_  = inChannels;
    outChannels_ = outChannels;
    rebuild();
}

void TrackHeaderList::rebuild() {
    // Clear existing header widgets (keep the trailing stretch).
    while (layout_->count() > 1) {
        QLayoutItem* item = layout_->takeAt(0);
        if (auto* w = item->widget()) w->deleteLater();
        delete item;
    }

    const int n = static_cast<int>(project_->tracks().size());
    for (int i = 0; i < n; ++i) {
        auto* h = new TrackHeader(project_, i, inChannels_, outChannels_);
        layout_->insertWidget(i, h);
    }
}

} // namespace notwork::gui
