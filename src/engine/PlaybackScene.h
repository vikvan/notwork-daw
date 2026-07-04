#pragma once

#include <cstdint>
#include <memory>
#include <vector>

namespace notwork::engine {

class AudioClip;

// Immutable snapshot handed to the RT callback. Rebuilt on any project
// change and published via std::atomic<std::shared_ptr<const PlaybackScene>>.
struct PlaybackClip {
    std::shared_ptr<const AudioClip> clip;
    int      outChannel   = 0;
    int64_t  startSample  = 0;   // position on the timeline
    int64_t  offsetSamples = 0;  // offset into the source clip
    int64_t  lengthSamples = 0;  // duration on the timeline
};

struct PlaybackScene {
    std::vector<PlaybackClip> clips;
};

} // namespace notwork::engine
