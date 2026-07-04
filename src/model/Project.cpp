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

} // namespace notwork::model
