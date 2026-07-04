#include "engine/Recorder.h"

#include <QDateTime>
#include <QDebug>
#include <QDir>

#include <sndfile.h>

#include <chrono>

namespace notwork::engine {

namespace {

constexpr std::size_t kRingCapacitySeconds = 4;
constexpr std::size_t kDrainChunkFrames    = 8192;
constexpr auto        kDrainInterval       = std::chrono::milliseconds(15);

} // namespace

Recorder::Recorder() = default;

Recorder::~Recorder() {
    if (running_.load(std::memory_order_relaxed) || writer_.joinable()) {
        stop();
    }
}

bool Recorder::start(int sampleRate,
                     int inputChannels,
                     int64_t startSample,
                     const std::vector<ArmedTrack>& armed,
                     unsigned long maxFramesPerCallback,
                     const QString& takesDir) {
    tracks_.clear();
    sampleRate_    = sampleRate;
    inputChannels_ = inputChannels;

    if (armed.empty() || inputChannels <= 0) return false;

    QDir().mkpath(takesDir);

    const QString stamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss_zzz");

    for (const auto& a : armed) {
        auto rt = std::make_unique<RecordTrack>();
        rt->trackIndex  = a.trackIndex;
        rt->inChannel   = a.inChannel;
        rt->startSample = startSample;
        rt->filePath    = QString("%1/track%2_%3.wav")
                              .arg(takesDir)
                              .arg(a.trackIndex + 1, 2, 10, QChar('0'))
                              .arg(stamp);

        SF_INFO info{};
        info.samplerate = sampleRate;
        info.channels   = 1;
        info.format     = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

        rt->snd = sf_open(rt->filePath.toUtf8().constData(), SFM_WRITE, &info);
        if (!rt->snd) {
            qWarning() << "sf_open failed for" << rt->filePath << ":" << sf_strerror(nullptr);
            continue;
        }

        rt->ring    = std::make_unique<SpscRingBuffer<float>>(
                          static_cast<std::size_t>(sampleRate) * kRingCapacitySeconds);
        rt->scratch.assign(maxFramesPerCallback, 0.0f);

        tracks_.push_back(std::move(rt));
    }

    if (tracks_.empty()) return false;

    running_.store(true, std::memory_order_release);
    writer_ = std::thread(&Recorder::writerLoop, this);
    return true;
}

void Recorder::pushInterleaved(const float* input, unsigned long frames) {
    if (!input) return;
    for (auto& rt : tracks_) {
        const int ch = rt->inChannel;
        if (ch < 0 || ch >= inputChannels_) continue;
        // Extract the channel into scratch, then push to ring.
        const unsigned long n = std::min<unsigned long>(frames,
                                    static_cast<unsigned long>(rt->scratch.size()));
        for (unsigned long f = 0; f < n; ++f) {
            rt->scratch[f] = input[f * inputChannels_ + ch];
        }
        rt->ring->push(rt->scratch.data(), n);
    }
}

std::vector<Recorder::Finalized> Recorder::stop() {
    running_.store(false, std::memory_order_release);
    if (writer_.joinable()) writer_.join();

    std::vector<Finalized> out;
    out.reserve(tracks_.size());
    for (auto& rt : tracks_) {
        if (rt->snd) {
            sf_close(rt->snd);
            rt->snd = nullptr;
        }
        Finalized f{};
        f.trackIndex    = rt->trackIndex;
        f.filePath      = rt->filePath;
        f.startSample   = rt->startSample;
        f.lengthSamples = rt->writtenSamples;
        out.push_back(f);
    }
    tracks_.clear();
    return out;
}

bool Recorder::anyPending() const {
    for (const auto& rt : tracks_) {
        if (rt->ring && rt->ring->readAvailable() > 0) return true;
    }
    return false;
}

void Recorder::writerLoop() {
    std::vector<float> buf(kDrainChunkFrames);
    while (running_.load(std::memory_order_acquire) || anyPending()) {
        bool wrote = false;
        for (auto& rt : tracks_) {
            if (!rt->snd || !rt->ring) continue;
            while (true) {
                const std::size_t got = rt->ring->pop(buf.data(), buf.size());
                if (got == 0) break;
                const sf_count_t n = sf_writef_float(rt->snd, buf.data(),
                                                     static_cast<sf_count_t>(got));
                rt->writtenSamples += static_cast<int64_t>(n);
                wrote = true;
            }
        }
        if (!wrote) std::this_thread::sleep_for(kDrainInterval);
    }
}

} // namespace notwork::engine
