
#ifndef PHYSICS_EVENT_DEFINE_HPP
#define PHYSICS_EVENT_DEFINE_HPP

#include <cstdint>

// Event type enum
enum class ePhysicsEventType : std::uint32_t
{
    none           = 0,  // null event
    collision      = 1   // collision detected
};

// Event: body struct
struct sPhysicsEventBody
{
    std::uint32_t componentID = 0;
    eBodyType    bodyType     = eBodyType::noneObject;
};

// Event struct
struct sPhysicsEvent
{
    sPhysicsEvent*    next    = nullptr;
    ePhysicsEventType type    = ePhysicsEventType::none;
    std::uint32_t     data    = 0;
    sPhysicsEventBody bodyA   = {};
    sPhysicsEventBody bodyB   = {};
};

#endif // PHYSICS_EVENT_DEFINE_HPP
