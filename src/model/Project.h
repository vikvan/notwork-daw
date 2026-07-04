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

    // GUI-triggered mutations use this to notify listeners.
    void notifyChanged() { emit changed(); }

signals:
    void changed();

private:
    int sampleRate_ = 48000;
    std::vector<Track> tracks_;
};

} // namespace notwork::model
