#pragma once

#include <QScrollArea>

class QVBoxLayout;

namespace notwork::model { class Project; }

namespace notwork::gui {

class TrackHeaderList : public QScrollArea {
    Q_OBJECT
public:
    TrackHeaderList(notwork::model::Project* project,
                    int inChannels,
                    int outChannels,
                    QWidget* parent = nullptr);

    void setChannels(int inChannels, int outChannels);

private:
    void rebuild();

    notwork::model::Project* project_;
    int inChannels_  = 0;
    int outChannels_ = 0;
    QWidget*     content_ = nullptr;
    QVBoxLayout* layout_  = nullptr;
};

} // namespace notwork::gui
