
#ifndef GRAPHICS_COMPONENT_DEFINE_HPP
#define GRAPHICS_COMPONENT_DEFINE_HPP

#include "animation_component_define.hpp"
#include "material_component_define.hpp"

struct sComponentGraphics
{
    bool enabled = false;

    // data
    glm::mat4                       modelMatrix = glm::mat4(1);
    glm::vec3                       scale       = glm::vec3(1.0f, 1.0f, 1.0f);
    std::int32_t                    model       = -1;
    std::vector<sComponentMaterial> material    = {};
};

#endif // GRAPHICS_COMPONENT_DEFINE_HPP
