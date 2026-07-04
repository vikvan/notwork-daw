#include "gui/TrackHeader.h"

#include "model/Project.h"
#include "model/Track.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPainter>
#include <QToolButton>
#include <QVBoxLayout>

namespace notwork::gui {

namespace {

QToolButton* makeToggle(const QString& text, const QString& tip, const QString& accentColor) {
    auto* b = new QToolButton;
    b->setText(text);
    b->setToolTip(tip);
    b->setCheckable(true);
    b->setFixedSize(20, 20);
    b->setStyleSheet(QString(
        "QToolButton { background: #2e2f33; border: 1px solid #3a3b3f; border-radius: 3px; color: #c8c9ce; font-weight: bold; }"
        "QToolButton:checked { background: %1; border-color: %1; color: white; }"
    ).arg(accentColor));
    return b;
}

} // namespace

TrackHeader::TrackHeader(notwork::model::Project* project,
                         int trackIndex,
                         int inChannels,
                         int outChannels,
                         QWidget* parent)
    : QFrame(parent), project_(project), trackIndex_(trackIndex) {
    setFixedHeight(kHeight);
    setAutoFillBackground(true);
    setObjectName("trackHeader");
    setStyleSheet("QFrame#trackHeader { background: #26272b; border-bottom: 1px solid #1a1b1e; }");

    nameEdit_ = new QLineEdit;
    nameEdit_->setStyleSheet(
        "QLineEdit { background: transparent; border: none; color: #dcddd0; font-weight: 600; }"
        "QLineEdit:focus { background: #1e1f22; border: 1px solid #3a7bd5; padding: 1px; }");

    armBtn_  = makeToggle("R", "Arm for recording", "#c8433a");
    muteBtn_ = makeToggle("M", "Mute",              "#d97706");
    soloBtn_ = makeToggle("S", "Solo",              "#3a7bd5");

    inCombo_  = new QComboBox;
    outCombo_ = new QComboBox;
    for (int i = 0; i < inChannels;  ++i) inCombo_ ->addItem(QString("In %1").arg(i + 1), i);
    for (int i = 0; i < outChannels; ++i) outCombo_->addItem(QString("Out %1").arg(i + 1), i);
    if (inChannels  == 0) inCombo_ ->addItem("—", -1);
    if (outChannels == 0) outCombo_->addItem("—", -1);
    for (auto* c : {inCombo_, outCombo_}) {
        c->setStyleSheet(
            "QComboBox { background: #2e2f33; border: 1px solid #3a3b3f; border-radius: 3px; "
            "color: #dcddd0; padding: 1px 4px; }");
        c->setMinimumWidth(72);
    }

    auto* row1 = new QHBoxLayout;
    row1->setContentsMargins(0, 0, 0, 0);
    row1->setSpacing(6);
    row1->addWidget(nameEdit_, 1);

    auto* row2 = new QHBoxLayout;
    row2->setContentsMargins(0, 0, 0, 0);
    row2->setSpacing(4);
    row2->addWidget(armBtn_);
    row2->addWidget(muteBtn_);
    row2->addWidget(soloBtn_);
    row2->addSpacing(6);
    row2->addWidget(inCombo_,  1);
    row2->addWidget(outCombo_, 1);

    auto* outer = new QVBoxLayout(this);
    outer->setContentsMargins(14, 6, 8, 6);   // 14 left leaves room for color stripe
    outer->setSpacing(4);
    outer->addLayout(row1);
    outer->addLayout(row2);

    refreshFromModel();

    // Wire mutations back into the model.
    connect(nameEdit_, &QLineEdit::editingFinished, this, [this]{
        project_->track(trackIndex_).name = nameEdit_->text();
        project_->notifyChanged();
    });
    auto pushFlags = [this]{
        auto& t = project_->track(trackIndex_);
        t.armed = armBtn_ ->isChecked();
        t.mute  = muteBtn_->isChecked();
        t.solo  = soloBtn_->isChecked();
        project_->notifyChanged();
    };
    connect(armBtn_,  &QToolButton::toggled, this, pushFlags);
    connect(muteBtn_, &QToolButton::toggled, this, pushFlags);
    connect(soloBtn_, &QToolButton::toggled, this, pushFlags);
    connect(inCombo_,  QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int){
        project_->track(trackIndex_).inCh = inCombo_->currentData().toInt();
        project_->notifyChanged();
    });
    connect(outCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int){
        project_->track(trackIndex_).outCh = outCombo_->currentData().toInt();
        project_->notifyChanged();
    });
}

void TrackHeader::refreshFromModel() {
    const auto& t = project_->track(trackIndex_);
    nameEdit_->setText(t.name);
    armBtn_ ->setChecked(t.armed);
    muteBtn_->setChecked(t.mute);
    soloBtn_->setChecked(t.solo);
    const int inIdx  = inCombo_ ->findData(t.inCh);
    const int outIdx = outCombo_->findData(t.outCh);
    if (inIdx  >= 0) inCombo_ ->setCurrentIndex(inIdx);
    if (outIdx >= 0) outCombo_->setCurrentIndex(outIdx);
}

void TrackHeader::paintEvent(QPaintEvent* e) {
    QFrame::paintEvent(e);
    QPainter p(this);
    p.fillRect(QRect(0, 0, 6, height()), project_->track(trackIndex_).color);
}

} // namespace notwork::gui
