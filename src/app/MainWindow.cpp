#include "app/MainWindow.h"

#include "app/SettingsDialog.h"
#include "engine/AudioEngine.h"
#include "gui/TimelineScene.h"
#include "gui/TimelineView.h"
#include "gui/TrackHeaderList.h"
#include "gui/TransportBar.h"
#include "model/Project.h"
#include "model/Track.h"

#include <QAction>
#include <QDialog>
#include <QKeySequence>
#include <QMenuBar>
#include <QScrollBar>
#include <QSplitter>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

namespace notwork::app {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      engine_(std::make_unique<engine::AudioEngine>()),
      project_(std::make_unique<model::Project>()) {
    setWindowTitle("Notwork");

    project_->setSampleRate(engine_->sampleRate());
    seedDemoProject();
    engine_->setProject(project_.get());

    auto* transport = new gui::TransportBar(engine_.get(), this);

    headerList_    = new gui::TrackHeaderList(project_.get(),
                                              engine_->inputChannels(),
                                              engine_->outputChannels(),
                                              this);
    headerList_->setMinimumWidth(240);
    headerList_->setMaximumWidth(320);

    timelineScene_ = new gui::TimelineScene(project_.get(), engine_.get(), this);
    timelineView_  = new gui::TimelineView(timelineScene_, this);

    auto* leftSpacer = new QWidget;
    leftSpacer->setFixedHeight(int(gui::TimelineScene::kRulerHeight));
    leftSpacer->setStyleSheet("background:#1c1d21; border-bottom:1px solid #121316;");
    auto* leftCol = new QWidget;
    auto* leftLay = new QVBoxLayout(leftCol);
    leftLay->setContentsMargins(0, 0, 0, 0);
    leftLay->setSpacing(0);
    leftLay->addWidget(leftSpacer);
    leftLay->addWidget(headerList_, 1);

    auto* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(leftCol);
    splitter->addWidget(timelineView_);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);
    splitter->setCollapsible(0, false);
    splitter->setCollapsible(1, false);
    splitter->setChildrenCollapsible(false);
    splitter->setHandleWidth(1);

    auto* central = new QWidget(this);
    auto* layout  = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(transport);
    layout->addWidget(splitter, 1);
    setCentralWidget(central);

    // Sync vertical scrolling between header list and timeline view.
    auto* headerVBar   = headerList_ ->verticalScrollBar();
    auto* timelineVBar = timelineView_->verticalScrollBar();
    connect(headerVBar,   &QScrollBar::valueChanged, timelineVBar,
            [timelineVBar](int v){ if (timelineVBar->value() != v) timelineVBar->setValue(v); });
    connect(timelineVBar, &QScrollBar::valueChanged, headerVBar,
            [headerVBar](int v){ if (headerVBar->value() != v) headerVBar->setValue(v); });

    auto* prefsAction = new QAction("Preferences…", this);
    prefsAction->setMenuRole(QAction::PreferencesRole);
    prefsAction->setShortcut(QKeySequence::Preferences);
    connect(prefsAction, &QAction::triggered, this, &MainWindow::openPreferences);

    auto* fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction(prefsAction);

    connect(engine_.get(), &engine::AudioEngine::configurationChanged, this, [this]{
        project_->setSampleRate(engine_->sampleRate());
        headerList_->setChannels(engine_->inputChannels(), engine_->outputChannels());
    });

    if (!engine_->lastError().isEmpty()) {
        statusBar()->showMessage(engine_->lastError());
    } else {
        statusBar()->showMessage("Ready");
    }
}

MainWindow::~MainWindow() = default;

void MainWindow::seedDemoProject() {
    const int sr = project_->sampleRate();
    project_->addTrack("Vocal",  QColor(58, 123, 213));
    project_->addTrack("Guitar", QColor(200,  67,  58));
    project_->addTrack("Bass",   QColor( 76, 175,  80));
    project_->addTrack("Drums",  QColor(217, 119,   6));

    model::AudioEvent e1;
    e1.name = "take 1";
    e1.startSamples  = 0;
    e1.lengthSamples = int64_t(sr) * 3;
    project_->addEvent(0, e1);

    model::AudioEvent e2;
    e2.name = "riff";
    e2.startSamples  = int64_t(sr) * 4;
    e2.lengthSamples = int64_t(sr) * 5;
    project_->addEvent(1, e2);

    model::AudioEvent e3;
    e3.name = "groove";
    e3.startSamples  = int64_t(sr) * 2;
    e3.lengthSamples = int64_t(sr) * 6;
    project_->addEvent(3, e3);
}

void MainWindow::openPreferences() {
    SettingsDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        engine_->reconfigure();
        if (!engine_->lastError().isEmpty()) {
            statusBar()->showMessage(engine_->lastError());
        } else {
            statusBar()->showMessage("Audio reconfigured", 2000);
        }
    }
}

} // namespace notwork::app
