
#ifndef MATERIAL_COMPONENT_DEFINE_HPP
#define MATERIAL_COMPONENT_DEFINE_HPP

#include <cstdint>
#include "texture_component_define.hpp"

struct sComponentMaterial
{
    std::int32_t diffuse  = -1;
    std::int32_t specular = -1;
    std::int32_t normal   = -1;
};

#endif // MATERIAL_COMPONENT_DEFINE_HPP

