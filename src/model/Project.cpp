#include "model/Project.h"

namespace notwork::model {

Project::Project(QObject* parent) : QObject(parent) {}

void Project::setSampleRate(int hz) {
    if (hz == sampleRate_) return;
    sampleRate_ = hz;
    emit changed();
}

int Project::addTrack(const QString& name, const QColor& color) {
    Track t;
    t.name  = name;
    t.color = color;
    tracks_.push_back(std::move(t));
    emit changed();
    return static_cast<int>(tracks_.size()) - 1;
}

void Project::addEvent(int trackIndex, AudioEvent ev) {
    tracks_[trackIndex].events.push_back(std::move(ev));
    emit changed();
}

void Project::loadState(int sampleRate, std::vector<Track> newTracks) {
    sampleRate_ = sampleRate;
    tracks_ = std::move(newTracks);
    emit changed();
}

void Project::moveEvent(int oldTrack, int oldIndex, int newTrack, int64_t newStartSamples) {
    if (oldTrack  < 0 || oldTrack  >= static_cast<int>(tracks_.size())) return;
    if (newTrack  < 0 || newTrack  >= static_cast<int>(tracks_.size())) return;
    auto& srcEvents = tracks_[oldTrack].events;
    if (oldIndex < 0 || oldIndex >= static_cast<int>(srcEvents.size())) return;

    if (oldTrack == newTrack) {
        srcEvents[oldIndex].startSamples = std::max<int64_t>(0, newStartSamples);
    } else {
        AudioEvent ev = std::move(srcEvents[oldIndex]);
        ev.startSamples = std::max<int64_t>(0, newStartSamples);
        srcEvents.erase(srcEvents.begin() + oldIndex);
        tracks_[newTrack].events.push_back(std::move(ev));
    }

    // Queued emit: keeps the ClipItem being dragged alive through
    // mouseReleaseEvent — the scene rebuild that follows would delete it.
    QMetaObject::invokeMethod(this, [this]{ emit changed(); },
                              Qt::QueuedConnection);
}

} // namespace notwork::model
