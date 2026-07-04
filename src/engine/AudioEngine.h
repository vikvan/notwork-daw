#pragma once

#include <QObject>
#include <QString>

#include <atomic>
#include <cstdint>
#include <memory>

#include <portaudio.h>

namespace notwork::model  { class Project; }

namespace notwork::engine {

class Recorder;

enum class TransportState : int {
    Stopped   = 0,
    Playing   = 1,
    Recording = 2,
};

// Owns a duplex PortAudio stream.
// Public API is called from the GUI thread. The audio callback runs on
// PortAudio's RT thread and only touches atomics.
class AudioEngine : public QObject {
    Q_OBJECT
public:
    explicit AudioEngine(QObject* parent = nullptr);
    ~AudioEngine() override;

    // Open (or re-open) the stream using current Settings. Returns true on success.
    bool reconfigure();

    void setProject(notwork::model::Project* project) { project_ = project; }

    void startPlayback();
    void startRecording();
    void stop();
    void rewind();

    TransportState state() const { return state_.load(std::memory_order_acquire); }
    int64_t        playheadSamples() const { return playhead_.load(std::memory_order_acquire); }
    int            sampleRate()     const { return sampleRate_; }
    int            inputChannels()  const { return inChannels_; }
    int            outputChannels() const { return outChannels_; }

    QString lastError() const { return lastError_; }

signals:
    void configurationChanged();

private:
    static int paCallback(const void* input, void* output,
                          unsigned long frameCount,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void* userData);
    int onCallback(const float* in, float* out, unsigned long frames);

    void closeStream();

    PaStream*   stream_    = nullptr;
    int         sampleRate_ = 48000;
    int         bufferSize_ = 512;
    int         inChannels_  = 0;
    int         outChannels_ = 0;

    std::atomic<TransportState> state_{TransportState::Stopped};
    std::atomic<int64_t>        playhead_{0};

    notwork::model::Project*    project_ = nullptr;
    std::unique_ptr<Recorder>   recorder_;

    QString lastError_;
};

} // namespace notwork::engine
