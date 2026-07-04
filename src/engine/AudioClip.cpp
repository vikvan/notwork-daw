#include "engine/AudioClip.h"

#include <QDebug>

#include <sndfile.h>

#include <unordered_map>

namespace notwork::engine {

namespace {

// Simple GUI-thread-only cache. Keys off absolute paths.
std::unordered_map<QString, std::weak_ptr<const AudioClip>>& cacheMap() {
    static std::unordered_map<QString, std::weak_ptr<const AudioClip>> c;
    return c;
}

} // namespace

std::shared_ptr<AudioClip> AudioClip::readFile(const QString& path) {
    SF_INFO info{};
    SNDFILE* snd = sf_open(path.toUtf8().constData(), SFM_READ, &info);
    if (!snd) {
        qWarning() << "AudioClip: sf_open failed for" << path
                   << ":" << sf_strerror(nullptr);
        return nullptr;
    }

    auto clip = std::shared_ptr<AudioClip>(new AudioClip);
    clip->channels_   = info.channels;
    clip->sampleRate_ = info.samplerate;
    clip->frames_     = info.frames;
    clip->data_.resize(static_cast<std::size_t>(info.frames) * info.channels);

    const sf_count_t read = sf_readf_float(snd, clip->data_.data(), info.frames);
    sf_close(snd);

    if (read != info.frames) {
        qWarning() << "AudioClip: sf_readf_float returned" << read
                   << "of" << info.frames << "for" << path;
        clip->frames_ = read;
        clip->data_.resize(static_cast<std::size_t>(read) * info.channels);
    }
    return clip;
}

std::shared_ptr<const AudioClip> AudioClip::loadCached(const QString& path) {
    if (path.isEmpty()) return nullptr;

    auto& map = cacheMap();
    if (auto it = map.find(path); it != map.end()) {
        if (auto sp = it->second.lock()) return sp;
    }
    auto sp = AudioClip::readFile(path);
    if (sp) map[path] = sp;
    return sp;
}

} // namespace notwork::engine
