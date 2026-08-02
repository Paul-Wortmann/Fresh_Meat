
#ifndef PLAYER_EVENT_DEFINE_HPP
#define PLAYER_EVENT_DEFINE_HPP

#include <cstdint>

// Event type enum
enum class ePlayerEventType : std::uint32_t
{
    none           = 0,  // null event
    tileChange     = 1,  // tile change
    positionChange = 2   // position change
};

// Event struct
struct sPlayerEvent
{
    sPlayerEvent*    next    = nullptr;
    ePlayerEventType type    = ePlayerEventType::none;
    std::uint32_t    data    = 0;
};

#endif // PLAYER_EVENT_DEFINE_HPP
