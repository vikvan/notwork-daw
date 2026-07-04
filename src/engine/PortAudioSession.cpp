#include "engine/PortAudioSession.h"

#include <portaudio.h>
#include <QDebug>

namespace notwork::engine {

PortAudioSession::PortAudioSession() {
    const PaError err = Pa_Initialize();
    if (err != paNoError) {
        qWarning() << "Pa_Initialize failed:" << Pa_GetErrorText(err);
    }
}

PortAudioSession::~PortAudioSession() {
    Pa_Terminate();
}

} // namespace notwork::engine
