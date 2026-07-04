#include "engine/AudioEngine.h"

#include "app/Settings.h"
#include "engine/DeviceList.h"

#include <portaudio.h>

#include <QDebug>

#include <cstring>

namespace notwork::engine {

AudioEngine::AudioEngine(QObject* parent) : QObject(parent) {
    reconfigure();
}

AudioEngine::~AudioEngine() {
    closeStream();
}

void AudioEngine::closeStream() {
    if (stream_) {
        Pa_StopStream(stream_);
        Pa_CloseStream(stream_);
        stream_ = nullptr;
    }
}

bool AudioEngine::reconfigure() {
    closeStream();
    state_.store(TransportState::Stopped, std::memory_order_release);
    playhead_.store(0, std::memory_order_release);

    const app::Settings s;
    sampleRate_ = s.sampleRate();
    bufferSize_ = s.bufferSize();

    DeviceList devices;
    auto inDev  = devices.findInputByName (s.inputDeviceName());
    if (!inDev)  inDev  = devices.defaultInput();
    auto outDev = devices.findOutputByName(s.outputDeviceName());
    if (!outDev) outDev = devices.defaultOutput();

    if (!inDev || !outDev) {
        lastError_ = "No input/output device available";
        qWarning() << lastError_;
        emit configurationChanged();
        return false;
    }

    inChannels_  = inDev ->maxInputChannels;
    outChannels_ = outDev->maxOutputChannels;

    PaStreamParameters inParams{};
    inParams.device                    = inDev->paIndex;
    inParams.channelCount              = inChannels_;
    inParams.sampleFormat              = paFloat32;
    inParams.suggestedLatency          = Pa_GetDeviceInfo(inDev->paIndex)->defaultLowInputLatency;
    inParams.hostApiSpecificStreamInfo = nullptr;

    PaStreamParameters outParams{};
    outParams.device                    = outDev->paIndex;
    outParams.channelCount              = outChannels_;
    outParams.sampleFormat              = paFloat32;
    outParams.suggestedLatency          = Pa_GetDeviceInfo(outDev->paIndex)->defaultLowOutputLatency;
    outParams.hostApiSpecificStreamInfo = nullptr;

    const PaError err = Pa_OpenStream(
        &stream_,
        &inParams,
        &outParams,
        static_cast<double>(sampleRate_),
        static_cast<unsigned long>(bufferSize_),
        paNoFlag,
        &AudioEngine::paCallback,
        this);

    if (err != paNoError) {
        lastError_ = QString("Pa_OpenStream failed: %1").arg(Pa_GetErrorText(err));
        qWarning() << lastError_;
        stream_ = nullptr;
        emit configurationChanged();
        return false;
    }

    const PaError startErr = Pa_StartStream(stream_);
    if (startErr != paNoError) {
        lastError_ = QString("Pa_StartStream failed: %1").arg(Pa_GetErrorText(startErr));
        qWarning() << lastError_;
        Pa_CloseStream(stream_);
        stream_ = nullptr;
        emit configurationChanged();
        return false;
    }

    lastError_.clear();
    qInfo() << "Audio stream open:"
            << inDev->name  << inChannels_  << "in,"
            << outDev->name << outChannels_ << "out,"
            << sampleRate_  << "Hz," << bufferSize_ << "frames";
    emit configurationChanged();
    return true;
}

void AudioEngine::startPlayback() {
    state_.store(TransportState::Playing,  std::memory_order_release);
}
void AudioEngine::startRecording() {
    state_.store(TransportState::Recording, std::memory_order_release);
}
void AudioEngine::stop() {
    state_.store(TransportState::Stopped,   std::memory_order_release);
}
void AudioEngine::rewind() {
    stop();
    playhead_.store(0, std::memory_order_release);
}

int AudioEngine::paCallback(const void* input, void* output,
                            unsigned long frameCount,
                            const PaStreamCallbackTimeInfo* /*timeInfo*/,
                            PaStreamCallbackFlags /*statusFlags*/,
                            void* userData) {
    return static_cast<AudioEngine*>(userData)->onCallback(
        static_cast<const float*>(input),
        static_cast<float*>(output),
        frameCount);
}

int AudioEngine::onCallback(const float* /*in*/, float* out, unsigned long frames) {
    // MVP step 3: no mixing yet, just silence.
    std::memset(out, 0, sizeof(float) * frames * static_cast<size_t>(outChannels_));

    if (state_.load(std::memory_order_acquire) != TransportState::Stopped) {
        playhead_.fetch_add(static_cast<int64_t>(frames), std::memory_order_acq_rel);
    }
    return paContinue;
}

} // namespace notwork::engine
