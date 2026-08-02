
#ifndef MAP_EVENT_DEFINE_HPP
#define MAP_EVENT_DEFINE_HPP

#include <cstdint>

// Event type enum
enum class eMapEventType : std::uint32_t
{
    none        = 0,  // null event
    tileClicked = 1,
    pathChanged = 2,  // path changed
    portal      = 3,  // Player triggered a portal tile
    bossAlert   = 4   // Player triggered a boss alert tile
};

// Event data struct
struct sMapEventData
{
    std::uint32_t tileX     = 0;
    std::uint32_t tileZ     = 0;
    std::uint32_t tileIndex = 0;
    std::uint32_t tileType  = 0;
};

// Event struct
struct sMapEvent
{
    sMapEvent*    next    = nullptr;
    eMapEventType type    = eMapEventType::none;
    sMapEventData data    = {};
};

#endif // MAP_EVENT_DEFINE_HPP
