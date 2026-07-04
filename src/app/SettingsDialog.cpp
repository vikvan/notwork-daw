#include "app/SettingsDialog.h"

#include "app/Settings.h"
#include "engine/DeviceList.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QVBoxLayout>

namespace notwork::app {

namespace {

const std::vector<int> kSampleRates = {44100, 48000, 88200, 96000};
const std::vector<int> kBufferSizes = {64, 128, 256, 512, 1024, 2048};

int indexOfOrAppend(QComboBox* combo, const QString& value) {
    const int idx = combo->findText(value);
    if (idx >= 0) return idx;
    combo->addItem(value);
    return combo->count() - 1;
}

} // namespace

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Preferences");
    setModal(true);

    sampleRateCombo_ = new QComboBox(this);
    for (int sr : kSampleRates) sampleRateCombo_->addItem(QString::number(sr), sr);

    bufferSizeCombo_ = new QComboBox(this);
    for (int bs : kBufferSizes) bufferSizeCombo_->addItem(QString::number(bs), bs);

    inputCombo_  = new QComboBox(this);
    outputCombo_ = new QComboBox(this);

    engine::DeviceList devices;
    for (const auto& d : devices.inputs())  inputCombo_->addItem(d.name);
    for (const auto& d : devices.outputs()) outputCombo_->addItem(d.name);

    auto* form = new QFormLayout;
    form->addRow("Sample Rate (Hz):", sampleRateCombo_);
    form->addRow("Buffer Size (frames):", bufferSizeCombo_);
    form->addRow("Input Device:",  inputCombo_);
    form->addRow("Output Device:", outputCombo_);

    auto* buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
        saveToSettings();
        accept();
    });
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

    auto* main = new QVBoxLayout(this);
    main->addLayout(form);
    main->addWidget(buttons);
    setMinimumWidth(420);

    loadFromSettings();
}

void SettingsDialog::loadFromSettings() {
    const Settings s;

    const int srIdx = sampleRateCombo_->findData(s.sampleRate());
    sampleRateCombo_->setCurrentIndex(
        srIdx >= 0 ? srIdx : sampleRateCombo_->findData(Settings::kDefaultSampleRate));

    const int bsIdx = bufferSizeCombo_->findData(s.bufferSize());
    bufferSizeCombo_->setCurrentIndex(
        bsIdx >= 0 ? bsIdx : bufferSizeCombo_->findData(Settings::kDefaultBufferSize));

    QString inName  = s.inputDeviceName();
    QString outName = s.outputDeviceName();

    engine::DeviceList devices;
    if (inName.isEmpty()) {
        if (auto d = devices.defaultInput())  inName  = d->name;
    }
    if (outName.isEmpty()) {
        if (auto d = devices.defaultOutput()) outName = d->name;
    }
    if (!inName.isEmpty())  inputCombo_ ->setCurrentIndex(indexOfOrAppend(inputCombo_,  inName));
    if (!outName.isEmpty()) outputCombo_->setCurrentIndex(indexOfOrAppend(outputCombo_, outName));
}

void SettingsDialog::saveToSettings() {
    Settings s;
    s.setSampleRate     (sampleRateCombo_->currentData().toInt());
    s.setBufferSize     (bufferSizeCombo_->currentData().toInt());
    s.setInputDeviceName (inputCombo_ ->currentText());
    s.setOutputDeviceName(outputCombo_->currentText());
}

} // namespace notwork::app
