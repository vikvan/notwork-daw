#pragma once

#include <QString>
#include <optional>
#include <vector>

namespace notwork::engine {

struct DeviceInfo {
    int paIndex = -1;
    QString name;
    int maxInputChannels = 0;
    int maxOutputChannels = 0;
    double defaultSampleRate = 0.0;
};

// Snapshot of PortAudio devices at construction time.
// Requires PortAudioSession to be alive.
class DeviceList {
public:
    DeviceList();

    const std::vector<DeviceInfo>& inputs()  const { return inputs_;  }
    const std::vector<DeviceInfo>& outputs() const { return outputs_; }

    std::optional<DeviceInfo> findInputByName(const QString& name)  const;
    std::optional<DeviceInfo> findOutputByName(const QString& name) const;

    // PortAudio "default" devices, if any.
    std::optional<DeviceInfo> defaultInput()  const;
    std::optional<DeviceInfo> defaultOutput() const;

private:
    std::vector<DeviceInfo> inputs_;
    std::vector<DeviceInfo> outputs_;
    int defaultInputIndex_  = -1;
    int defaultOutputIndex_ = -1;
};

} // namespace notwork::engine
