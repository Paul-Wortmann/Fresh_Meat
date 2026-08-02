

#ifndef AUDIO_EVENT_DEFINE_HPP
#define AUDIO_EVENT_DEFINE_HPP

#include <cstdint>

// Event type enum
enum eAudioEventType : std::uint32_t
{
    audioEventType_none         = 0     // null event
};

// Event struct
struct sAudioEvent
{
    sAudioEvent*    next = nullptr;
    eAudioEventType type = eAudioEventType::audioEventType_none;
    std::uint32_t   data = 0;
};

#endif // AUDIO_EVENT_DEFINE_HPP

