

#ifndef MODEL_DEFINE_HPP
#define MODEL_DEFINE_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "model_component_define.hpp" // for sNode, sSkin, sAnimation

struct sVertexData
{
    glm::vec3  position  = {};
    glm::vec3  normal    = {};
    glm::vec2  texCoord  = {};
    glm::vec4  tangent   = {};
    glm::ivec4 joints    = glm::ivec4(0);   // joint indices (max 4)
    glm::vec4  weights   = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f); // weights (sum = 1)
};

struct sMeshData
{
    bool                       enabled = {};
    std::string                name    = {};
    std::vector<sVertexData>   vertex  = {};
    std::vector<std::uint32_t> index   = {};
};

struct sModelData
{
    std::string             name      = {};

    // Meshes
    std::vector<sMeshData>  mesh      = {};

    // Skeletal data
    std::vector<sNode>      node      = {};
    sSkin                   skin      = {};
    std::vector<sAnimation> animation = {};
    int                     rootNode  = -1;
};

void generateTangents(sModelData &_model);

#endif // MODEL_DEFINE_HPP

