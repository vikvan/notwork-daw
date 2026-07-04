#pragma once

#include <QString>
#include <cstdint>

namespace notwork::model {

// A single clip placed on a track lane. Coordinates are in samples at the
// project's sample rate.
struct AudioEvent {
    QString name;
    QString filePath;       // Empty for placeholder events (no audio yet).
    int64_t startSamples  = 0;
    int64_t offsetSamples = 0;
    int64_t lengthSamples = 0;
};

} // namespace notwork::model
