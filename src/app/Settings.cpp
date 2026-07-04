#include "app/Settings.h"

#include <QSettings>

namespace notwork::app {

namespace {
constexpr auto kSampleRate  = "audio/sampleRate";
constexpr auto kBufferSize  = "audio/bufferSize";
constexpr auto kInputName   = "audio/inputDeviceName";
constexpr auto kOutputName  = "audio/outputDeviceName";
} // namespace

int Settings::sampleRate() const {
    return QSettings{}.value(kSampleRate, kDefaultSampleRate).toInt();
}
void Settings::setSampleRate(int hz) {
    QSettings{}.setValue(kSampleRate, hz);
}

int Settings::bufferSize() const {
    return QSettings{}.value(kBufferSize, kDefaultBufferSize).toInt();
}
void Settings::setBufferSize(int frames) {
    QSettings{}.setValue(kBufferSize, frames);
}

QString Settings::inputDeviceName() const {
    return QSettings{}.value(kInputName).toString();
}
void Settings::setInputDeviceName(const QString& name) {
    QSettings{}.setValue(kInputName, name);
}

QString Settings::outputDeviceName() const {
    return QSettings{}.value(kOutputName).toString();
}
void Settings::setOutputDeviceName(const QString& name) {
    QSettings{}.setValue(kOutputName, name);
}

} // namespace notwork::app
