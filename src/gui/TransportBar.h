#pragma once

#include <QWidget>

class QLabel;
class QPushButton;
class QTimer;

namespace notwork::engine { class AudioEngine; }

namespace notwork::gui {

class TransportBar : public QWidget {
    Q_OBJECT
public:
    explicit TransportBar(engine::AudioEngine* engine, QWidget* parent = nullptr);

private:
    void refresh();

    engine::AudioEngine* engine_ = nullptr;

    QPushButton* rewindBtn_ = nullptr;
    QPushButton* playBtn_   = nullptr;
    QPushButton* stopBtn_   = nullptr;
    QPushButton* recBtn_    = nullptr;

    QLabel*  timeLabel_ = nullptr;
    QTimer*  uiTimer_   = nullptr;
};

} // namespace notwork::gui
