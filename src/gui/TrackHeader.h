#pragma once

#include <QFrame>

class QComboBox;
class QLineEdit;
class QToolButton;

namespace notwork::model { struct Track; class Project; }

namespace notwork::gui {

class TrackHeader : public QFrame {
    Q_OBJECT
public:
    TrackHeader(notwork::model::Project* project,
                int trackIndex,
                int inChannels,
                int outChannels,
                QWidget* parent = nullptr);

    static constexpr int kHeight = 72;

protected:
    void paintEvent(QPaintEvent* e) override;

private:
    void refreshFromModel();

    notwork::model::Project* project_;
    int trackIndex_;

    QLineEdit*   nameEdit_ = nullptr;
    QToolButton* armBtn_   = nullptr;
    QToolButton* muteBtn_  = nullptr;
    QToolButton* soloBtn_  = nullptr;
    QComboBox*   inCombo_  = nullptr;
    QComboBox*   outCombo_ = nullptr;
};

} // namespace notwork::gui
