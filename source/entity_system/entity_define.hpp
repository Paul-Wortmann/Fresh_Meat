#ifndef ENTITY_DEFINE_HPP
#define ENTITY_DEFINE_HPP

#include <cstdint>
#include <string>
#include <vector>

enum class eEntityType : std::uint32_t
{
    none          = 0,
    player        = 1,
    npc           = 2,
    objectStatic  = 3,
    objectDynamic = 4
};

struct sEntity
{
    bool         enabled = false;
    eEntityType  type    = eEntityType::none;

    // IDs are indexes into system vectors
    std::int32_t audioComponent    = -1;
    std::int32_t graphicsComponent = -1;
    std::int32_t physicsComponent  = -1;
};

#endif // ENTITY_DEFINE_HPP
