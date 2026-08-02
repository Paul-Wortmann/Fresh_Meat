#ifndef PHYSICS_COMPONENT_DEFINE_HPP
#define PHYSICS_COMPONENT_DEFINE_HPP

#include <glm/glm.hpp>
#include <cstdint>

enum class eBodyType : std::uint32_t
{
    noneObject    = 0, // Physics does not apply
    staticObject  = 1, // immovable object
    dynamicObject = 2  // responds to collisions
};

enum class eShapeType : std::uint32_t
{
    circle = 0,
    aabb   = 1
};

struct sComponentPhysics
{
    // component
    bool enabled = false;

    // shape
    eShapeType shapeType = eShapeType::circle;
    float      radius    = 1.0f;   // circle radius, or AABB side length

    // movement (linear)
    glm::vec3 direction    = {};   // (not used in physics, kept for compatibility)
    glm::vec3 position     = {};
    glm::vec3 velocity     = {};
    glm::vec3 maxVelocity  = {};
    glm::vec3 acceleration = {};
    glm::vec3 deceleration = {};
    glm::vec3 maxAcceleration = {};   // maximum linear acceleration (units/s²)
    glm::vec3 maxDeceleration = {};   // maximum linear deceleration (units/s²)

    // rotation (around Y axis, radians)
    float angle           = 0.0f;
    float angularVelocity = 0.0f;
    // angular acceleration can be added if needed

    // collision & physics properties
    eBodyType bodyType     = eBodyType::noneObject;
    float     invMass      = 0.0f;   // 1 / mass (0 → infinite mass)
    float     invInertia   = 0.0f;   // 1 / moment of inertia
    float     restitution  = 0.5f;   // coefficient of restitution (bounciness) [0,1]
    float     friction     = 0.5f;   // kinetic friction coefficient [0,1]
};

#endif // PHYSICS_COMPONENT_DEFINE_HPP
