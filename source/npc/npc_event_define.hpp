
#ifndef NPC_EVENT_DEFINE_HPP
#define NPC_EVENT_DEFINE_HPP

#include <cstdint>

enum class eNPCEventType : std::uint32_t
{
    none           = 0,
    positionChange = 1
};

struct sNPCEvent
{
    sNPCEvent*    next        = nullptr;
    eNPCEventType type        = eNPCEventType::none;
    std::uint32_t dataEntity  = 0; // Entity index
    std::uint32_t dataPhysics = 0; // Physics component index
};

#endif // NPC_EVENT_DEFINE_HPP
