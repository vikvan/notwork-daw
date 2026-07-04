#pragma once

#include <QDialog>

class QComboBox;

namespace notwork::app {

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

private:
    void loadFromSettings();
    void saveToSettings();

    QComboBox* sampleRateCombo_ = nullptr;
    QComboBox* bufferSizeCombo_ = nullptr;
    QComboBox* inputCombo_      = nullptr;
    QComboBox* outputCombo_     = nullptr;
};

} // namespace notwork::app
