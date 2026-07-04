#include "app/MainWindow.h"

#include "app/SettingsDialog.h"
#include "engine/AudioEngine.h"
#include "gui/TransportBar.h"

#include <QAction>
#include <QDialog>
#include <QKeySequence>
#include <QLabel>
#include <QMenuBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QWidget>

namespace notwork::app {

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      engine_(std::make_unique<engine::AudioEngine>()) {
    setWindowTitle("Notwork");

    auto* transport = new gui::TransportBar(engine_.get(), this);

    auto* placeholder = new QLabel("Notwork — multitrack DAW");
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setObjectName("placeholder");

    auto* central = new QWidget(this);
    auto* layout  = new QVBoxLayout(central);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(transport);
    layout->addWidget(placeholder, 1);
    setCentralWidget(central);

    auto* prefsAction = new QAction("Preferences…", this);
    prefsAction->setMenuRole(QAction::PreferencesRole);
    prefsAction->setShortcut(QKeySequence::Preferences);
    connect(prefsAction, &QAction::triggered, this, &MainWindow::openPreferences);

    auto* fileMenu = menuBar()->addMenu("File");
    fileMenu->addAction(prefsAction);

    if (!engine_->lastError().isEmpty()) {
        statusBar()->showMessage(engine_->lastError());
    } else {
        statusBar()->showMessage("Ready");
    }
}

MainWindow::~MainWindow() = default;

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
