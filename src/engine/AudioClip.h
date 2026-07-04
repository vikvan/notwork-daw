#pragma once

#include <QString>

#include <cstdint>
#include <memory>
#include <vector>

namespace notwork::engine {

// Immutable, in-RAM copy of an audio file. Loaded via libsndfile.
// For MVP we keep interleaved float samples; playback (step 9) will
// dereference by frame/channel.
class AudioClip {
public:
    // Returns a shared, cached clip for `path`. Cache lookup is by absolute
    // filesystem path. Only meant to be called from the GUI thread.
    static std::shared_ptr<const AudioClip> loadCached(const QString& path);

    int      channels()   const { return channels_; }
    int      sampleRate() const { return sampleRate_; }
    int64_t  frames()     const { return frames_; }
    bool     valid()      const { return frames_ > 0 && !data_.empty(); }
    const std::vector<float>& data() const { return data_; }

private:
    AudioClip() = default;

    static std::shared_ptr<AudioClip> readFile(const QString& path);

    int     channels_   = 0;
    int     sampleRate_ = 0;
    int64_t frames_     = 0;
    std::vector<float> data_;   // interleaved
};

} // namespace notwork::engine
