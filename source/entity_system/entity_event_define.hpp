
#ifndef ENTITY_EVENT_DEFINE_HPP
#define ENTITY_EVENT_DEFINE_HPP

#include <cstdint>

enum class eEntityEventType : std::uint32_t
{
    none           = 0,
    playerLoaded   = 1,
    npcLoaded      = 2
};

struct sEntityEvent
{
    sEntityEvent*    next = nullptr;
    eEntityEventType type = eEntityEventType::none;
    std::uint32_t    data = 0;
};

#endif // ENTITY_EVENT_DEFINE_HPP
