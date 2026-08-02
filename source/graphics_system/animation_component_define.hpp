
#ifndef ANIMATION_COMPONENT_DEFINE_HPP
#define ANIMATION_COMPONENT_DEFINE_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

enum class eAnimationType : std::int32_t
{
    none = 0,
    idle = 1,
    walk = 2
};


struct sComponentAnimation
{
    std::int32_t idle = -1;
    std::int32_t walk = -1;
};

#endif // ANIMATION_COMPONENT_DEFINE_HPP
