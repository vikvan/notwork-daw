#include <QApplication>

#include "app/MainWindow.h"
#include "engine/PortAudioSession.h"
#include "gui/Style.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setOrganizationName("Notwork");
    QApplication::setOrganizationDomain("notwork.local");
    QApplication::setApplicationName("Notwork");

    notwork::engine::PortAudioSession audioSession;
    notwork::gui::applyDarkStyle(app);

    notwork::app::MainWindow window;
    window.resize(1280, 720);
    window.show();

    return app.exec();
}
