#pragma once

#include "model/Track.h"

#include <QObject>

#include <vector>

namespace notwork::model {

class Project : public QObject {
    Q_OBJECT
public:
    explicit Project(QObject* parent = nullptr);

    int  sampleRate() const { return sampleRate_; }
    void setSampleRate(int hz);

    const std::vector<Track>& tracks() const { return tracks_; }
    Track&                    track(int i)   { return tracks_[i]; }

    int  addTrack(const QString& name, const QColor& color);
    void addEvent(int trackIndex, AudioEvent ev);

    // Replaces the entire project state atomically. Used by ProjectIO.
    void loadState(int sampleRate, std::vector<Track> newTracks);

    // Move an event to (possibly) another track and a new start position.
    // Emits `changed` asynchronously (queued) so the caller can safely be
    // a QGraphicsItem being dragged — the scene rebuild won't happen until
    // control returns to the event loop.
    void moveEvent(int oldTrack, int oldIndex, int newTrack, int64_t newStartSamples);

    // GUI-triggered mutations use this to notify listeners.
    void notifyChanged() { emit changed(); }

signals:
    void changed();

private:
    int sampleRate_ = 48000;
    std::vector<Track> tracks_;
};

} // namespace notwork::model
