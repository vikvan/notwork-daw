#pragma once

#include <QMainWindow>

#include <memory>

namespace notwork::engine { class AudioEngine; }

namespace notwork::app {

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    void openPreferences();

    std::unique_ptr<engine::AudioEngine> engine_;
};

} // namespace notwork::app
