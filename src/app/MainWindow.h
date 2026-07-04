#pragma once

#include <QMainWindow>

#include <memory>

namespace notwork::engine { class AudioEngine; }
namespace notwork::model  { class Project;     }
namespace notwork::gui    { class TrackHeaderList; class TimelineView; class TimelineScene; }

namespace notwork::app {

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

private:
    void openPreferences();
    void seedDemoProject();

    void openProject();
    void saveProject();
    void saveProjectAs();
    bool saveTo(const QString& path);
    void loadFrom(const QString& path);

    std::unique_ptr<engine::AudioEngine> engine_;
    std::unique_ptr<model::Project>      project_;

    gui::TrackHeaderList* headerList_ = nullptr;
    gui::TimelineView*    timelineView_ = nullptr;
    gui::TimelineScene*   timelineScene_ = nullptr;

    QString currentProjectPath_;
};

} // namespace notwork::app
