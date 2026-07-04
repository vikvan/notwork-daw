#include "engine/DeviceList.h"

#include <portaudio.h>
#include <QDebug>

namespace notwork::engine {

DeviceList::DeviceList() {
    const int count = Pa_GetDeviceCount();
    if (count < 0) {
        qWarning() << "Pa_GetDeviceCount failed:" << Pa_GetErrorText(count);
        return;
    }

    defaultInputIndex_  = Pa_GetDefaultInputDevice();
    defaultOutputIndex_ = Pa_GetDefaultOutputDevice();

    for (int i = 0; i < count; ++i) {
        const PaDeviceInfo* d = Pa_GetDeviceInfo(i);
        if (!d) continue;

        DeviceInfo info{
            i,
            QString::fromUtf8(d->name),
            d->maxInputChannels,
            d->maxOutputChannels,
            d->defaultSampleRate,
        };

        if (info.maxInputChannels  > 0) inputs_.push_back(info);
        if (info.maxOutputChannels > 0) outputs_.push_back(info);
    }
}

std::optional<DeviceInfo> DeviceList::findInputByName(const QString& name) const {
    for (const auto& d : inputs_) {
        if (d.name == name) return d;
    }
    return std::nullopt;
}

std::optional<DeviceInfo> DeviceList::findOutputByName(const QString& name) const {
    for (const auto& d : outputs_) {
        if (d.name == name) return d;
    }
    return std::nullopt;
}

std::optional<DeviceInfo> DeviceList::defaultInput() const {
    for (const auto& d : inputs_) {
        if (d.paIndex == defaultInputIndex_) return d;
    }
    return std::nullopt;
}

std::optional<DeviceInfo> DeviceList::defaultOutput() const {
    for (const auto& d : outputs_) {
        if (d.paIndex == defaultOutputIndex_) return d;
    }
    return std::nullopt;
}

} // namespace notwork::engine
