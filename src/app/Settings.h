#pragma once

#include <QString>

namespace notwork::app {

class Settings {
public:
    static constexpr int    kDefaultSampleRate = 48000;
    static constexpr int    kDefaultBufferSize = 512;

    int     sampleRate() const;
    void    setSampleRate(int hz);

    int     bufferSize() const;
    void    setBufferSize(int frames);

    QString inputDeviceName() const;
    void    setInputDeviceName(const QString& name);

    QString outputDeviceName() const;
    void    setOutputDeviceName(const QString& name);
};

} // namespace notwork::app
