#include "gui/TransportBar.h"

#include "engine/AudioEngine.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

namespace notwork::gui {

namespace {

QString formatTime(int64_t samples, int sampleRate) {
    if (sampleRate <= 0) return "00:00.000";
    const double seconds = static_cast<double>(samples) / sampleRate;
    const int totalMs = static_cast<int>(seconds * 1000.0);
    const int mm     = totalMs / 60000;
    const int ss     = (totalMs / 1000) % 60;
    const int ms     = totalMs % 1000;
    return QString("%1:%2.%3")
        .arg(mm, 2, 10, QChar('0'))
        .arg(ss, 2, 10, QChar('0'))
        .arg(ms, 3, 10, QChar('0'));
}

QPushButton* makeTransportBtn(const QString& text, const QString& tooltip) {
    auto* b = new QPushButton(text);
    b->setToolTip(tooltip);
    b->setFixedSize(40, 28);
    return b;
}

} // namespace

TransportBar::TransportBar(engine::AudioEngine* engine, QWidget* parent)
    : QWidget(parent), engine_(engine) {
    setObjectName("transportBar");

    rewindBtn_ = makeTransportBtn("⏮", "Rewind");
    playBtn_   = makeTransportBtn("⏵", "Play");
    stopBtn_   = makeTransportBtn("⏹", "Stop");
    recBtn_    = makeTransportBtn("⏺", "Record");
    recBtn_->setObjectName("recBtn");

    timeLabel_ = new QLabel("00:00.000");
    timeLabel_->setObjectName("timeLabel");
    timeLabel_->setMinimumWidth(120);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(6);
    layout->addWidget(rewindBtn_);
    layout->addWidget(playBtn_);
    layout->addWidget(stopBtn_);
    layout->addWidget(recBtn_);
    layout->addSpacing(16);
    layout->addWidget(timeLabel_);
    layout->addStretch();

    connect(rewindBtn_, &QPushButton::clicked, this, [this]{ engine_->rewind();          refresh(); });
    connect(playBtn_,   &QPushButton::clicked, this, [this]{ engine_->startPlayback();   refresh(); });
    connect(stopBtn_,   &QPushButton::clicked, this, [this]{ engine_->stop();            refresh(); });
    connect(recBtn_,    &QPushButton::clicked, this, [this]{ engine_->startRecording();  refresh(); });

    uiTimer_ = new QTimer(this);
    uiTimer_->setInterval(33); // ~30 Hz UI refresh
    connect(uiTimer_, &QTimer::timeout, this, &TransportBar::refresh);
    uiTimer_->start();
}

void TransportBar::refresh() {
    const auto state    = engine_->state();
    const auto samples  = engine_->playheadSamples();
    const int  sr       = engine_->sampleRate();

    timeLabel_->setText(formatTime(samples, sr));

    const bool recording = (state == engine::TransportState::Recording);
    const bool playing   = (state == engine::TransportState::Playing);
    recBtn_ ->setStyleSheet(recording
        ? "background-color: #c8433a; border: 1px solid #de5a4e; color: white;"
        : QString());
    playBtn_->setStyleSheet(playing
        ? "background-color: #3a7bd5; border: 1px solid #5a94e6; color: white;"
        : QString());
}

} // namespace notwork::gui
