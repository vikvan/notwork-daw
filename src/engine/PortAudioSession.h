#pragma once

namespace notwork::engine {

// RAII: Pa_Initialize on construction, Pa_Terminate on destruction.
// Exactly one instance should live for the lifetime of the app.
class PortAudioSession {
public:
    PortAudioSession();
    ~PortAudioSession();
    PortAudioSession(const PortAudioSession&) = delete;
    PortAudioSession& operator=(const PortAudioSession&) = delete;
};

} // namespace notwork::engine
