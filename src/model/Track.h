#pragma once

#include "model/AudioEvent.h"

#include <QColor>
#include <QString>

#include <vector>

namespace notwork::model {

struct Track {
    QString name;
    QColor  color = QColor(58, 123, 213);
    bool    armed = false;
    bool    mute  = false;
    bool    solo  = false;
    int     inCh  = 0;   // zero-based channel of the input device
    int     outCh = 0;   // zero-based channel of the output device
    std::vector<AudioEvent> events;
};

} // namespace notwork::model
