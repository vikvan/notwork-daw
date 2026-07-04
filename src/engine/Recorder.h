#pragma once

#include "engine/RingBuffer.h"

#include <QString>

#include <sndfile.h>

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>
#include <vector>

namespace notwork::engine {

// Multi-track recorder: one WAV writer per armed track, fed by lock-free
// ring buffers from the audio callback. A background thread drains the
// rings and writes them to disk via libsndfile.
class Recorder {
public:
    struct ArmedTrack {
        int trackIndex;   // model track index
        int inChannel;    // channel on the input device
    };

    struct Finalized {
        int     trackIndex;
        QString filePath;
        int64_t startSample;
        int64_t lengthSamples;
    };

    Recorder();
    ~Recorder();

    // Called from the GUI thread while audio state is Stopped.
    // Opens one WAV per armed track and spawns the writer thread.
    bool start(int sampleRate,
               int inputChannels,
               int64_t startSample,
               const std::vector<ArmedTrack>& armed,
               unsigned long maxFramesPerCallback,
               const QString& takesDir);

    // Called from the audio callback. Interleaved input has `inputChannels`
    // channels; extracts the configured channel of each armed track and
    // pushes into its ring.
    void pushInterleaved(const float* input, unsigned long frames);

    // Called from the GUI thread. Signals the writer to drain, joins it,
    // closes all WAVs and returns per-track metadata for events.
    std::vector<Finalized> stop();

private:
    struct RecordTrack {
        int      trackIndex   = 0;
        int      inChannel    = 0;
        int64_t  startSample  = 0;
        int64_t  writtenSamples = 0;
        QString  filePath;
        SNDFILE* snd          = nullptr;
        std::unique_ptr<SpscRingBuffer<float>> ring;
        std::vector<float> scratch;   // pre-allocated for the callback
    };

    void writerLoop();
    bool anyPending() const;

    std::vector<std::unique_ptr<RecordTrack>> tracks_;
    int inputChannels_ = 0;
    int sampleRate_    = 48000;

    std::thread       writer_;
    std::atomic<bool> running_{false};
};

} // namespace notwork::engine
