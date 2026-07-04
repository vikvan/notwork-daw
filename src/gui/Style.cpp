#include "gui/Style.h"

#include <QApplication>
#include <QColor>
#include <QPalette>

namespace notwork::gui {

void applyDarkStyle(QApplication& app) {
    app.setStyle("Fusion");

    QPalette palette;
    const QColor bg(30, 31, 34);
    const QColor bgAlt(38, 39, 43);
    const QColor bgLight(46, 47, 51);
    const QColor text(220, 221, 224);
    const QColor textDim(150, 152, 158);
    const QColor accent(58, 123, 213);

    palette.setColor(QPalette::Window, bg);
    palette.setColor(QPalette::WindowText, text);
    palette.setColor(QPalette::Base, bgAlt);
    palette.setColor(QPalette::AlternateBase, bgLight);
    palette.setColor(QPalette::ToolTipBase, bgLight);
    palette.setColor(QPalette::ToolTipText, text);
    palette.setColor(QPalette::Text, text);
    palette.setColor(QPalette::Button, bgLight);
    palette.setColor(QPalette::ButtonText, text);
    palette.setColor(QPalette::BrightText, Qt::red);
    palette.setColor(QPalette::Link, accent);
    palette.setColor(QPalette::Highlight, accent);
    palette.setColor(QPalette::HighlightedText, Qt::white);
    palette.setColor(QPalette::PlaceholderText, textDim);
    palette.setColor(QPalette::Disabled, QPalette::Text, textDim);
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, textDim);

    app.setPalette(palette);

    app.setStyleSheet(R"(
        QMainWindow, QDialog { background-color: #1e1f22; }
        QStatusBar { background-color: #26272b; color: #96989e; }
        QLabel#placeholder { color: #96989e; font-size: 18px; }
        QPushButton {
            background-color: #2e2f33;
            border: 1px solid #3a3b3f;
            padding: 4px 10px;
            border-radius: 3px;
        }
        QPushButton:hover  { background-color: #3a3b3f; }
        QPushButton:pressed{ background-color: #3a7bd5; }
    )");
}

} // namespace notwork::gui
