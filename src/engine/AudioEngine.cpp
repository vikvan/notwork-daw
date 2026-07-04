#include "engine/AudioEngine.h"

#include "app/Settings.h"
#include "engine/DeviceList.h"
#include "engine/Recorder.h"
#include "model/AudioEvent.h"
#include "model/Project.h"
#include "model/Track.h"

#include <portaudio.h>

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

#include <cstring>

namespace notwork::engine {

AudioEngine::AudioEngine(QObject* parent)
    : QObject(parent), recorder_(std::make_unique<Recorder>()) {
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
    state_.store(TransportState::Playing, std::memory_order_release);
}

void AudioEngine::startRecording() {
    if (!project_) {
        state_.store(TransportState::Playing, std::memory_order_release);
        return;
    }

    std::vector<Recorder::ArmedTrack> armed;
    for (int i = 0; i < static_cast<int>(project_->tracks().size()); ++i) {
        const auto& t = project_->track(i);
        if (t.armed && t.inCh >= 0 && t.inCh < inChannels_) {
            armed.push_back({i, t.inCh});
        }
    }

    if (armed.empty()) {
        // Nothing to record — behave like plain playback.
        state_.store(TransportState::Playing, std::memory_order_release);
        return;
    }

    const QString base = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    const QString takesDir = base + "/Notwork/untitled/audio";
    const int64_t startSample = playhead_.load(std::memory_order_acquire);

    if (!recorder_->start(sampleRate_, inChannels_,
                          startSample, armed,
                          static_cast<unsigned long>(bufferSize_),
                          takesDir)) {
        qWarning() << "Recorder::start failed";
        state_.store(TransportState::Playing, std::memory_order_release);
        return;
    }
    state_.store(TransportState::Recording, std::memory_order_release);
}

void AudioEngine::stop() {
    const TransportState prev =
        state_.exchange(TransportState::Stopped, std::memory_order_acq_rel);

    if (prev == TransportState::Recording && recorder_) {
        // Give the RT thread a beat to see the state change and stop pushing.
        Pa_Sleep(20);
        const auto results = recorder_->stop();
        if (project_) {
            for (const auto& r : results) {
                if (r.lengthSamples <= 0) continue;
                model::AudioEvent ev;
                ev.name          = QFileInfo(r.filePath).completeBaseName();
                ev.filePath      = r.filePath;
                ev.startSamples  = r.startSample;
                ev.lengthSamples = r.lengthSamples;
                project_->addEvent(r.trackIndex, ev);
            }
        }
    }
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

int AudioEngine::onCallback(const float* in, float* out, unsigned long frames) {
    // Silence output for now — mixing lands in a later step.
    std::memset(out, 0, sizeof(float) * frames * static_cast<size_t>(outChannels_));

    const TransportState st = state_.load(std::memory_order_acquire);
    if (st == TransportState::Recording && in && recorder_) {
        recorder_->pushInterleaved(in, frames);
    }
    if (st != TransportState::Stopped) {
        playhead_.fetch_add(static_cast<int64_t>(frames), std::memory_order_acq_rel);
    }
    return paContinue;
}

} // namespace notwork::engine
