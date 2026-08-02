
#ifndef MODEL_COMPONENT_DEFINE_HPP
#define MODEL_COMPONENT_DEFINE_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "animation_component_define.hpp"

struct sSkin
{
    std::vector<int> joints;                // node indices of joints
    std::vector<glm::mat4> inverseBindMatrices; // per-joint inverse bind matrices
};

struct sAnimationSampler
{
    int input;      // accessor index for times
    int output;     // accessor index for values
    std::string interpolation; // "LINEAR", "STEP", "CUBICSPLINE"

    // Pre‑converted keyframe data
    std::vector<float> times;
    std::vector<glm::vec3> translations;   // if output is vec3 and target is translation
    std::vector<glm::quat> rotations;      // if output is vec4 and target is rotation
    std::vector<glm::vec3> scales;         // if output is vec3 and target is scale
};

enum class eAnimTargetPath
{
    translation,
    rotation,
    scale
};

struct sAnimationChannel
{
    int sampler;            // index into samplers
    int targetNode;         // node index
    eAnimTargetPath targetPath; // "translation", "rotation", "scale", "weights"
};

struct sAnimation
{
    std::string name;
    std::vector<sAnimationSampler> samplers;
    std::vector<sAnimationChannel> channels;
    float duration = 0.0f;
};

struct sNode
{
    std::vector<int> children;
    glm::vec3 translation = glm::vec3(0.0f);
    glm::quat rotation    = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); // quaternion
    glm::vec3 scale       = glm::vec3(1.0f);
    glm::mat4 localMatrix = glm::mat4(1.0f);
};

struct sMesh
{
    std::string    name       = {};
    bool           enabled    = true;
    std::uint32_t  VAO        = 0;
    std::uint32_t  VBO        = 0;
    std::uint32_t  EBO        = 0;
    std::uint32_t  numElement = 0;
};

struct sComponentModel
{
    // component management
    std::string        fileName   = "";
    bool               enabled    = false;

    // graphics
    std::vector<sMesh> mesh = {};

    // animation
    sSkin skin;
    std::vector<sAnimation> animation      = {};
    std::vector<sNode>      node           = {};
    int                     rootNode       = -1;
    sComponentAnimation     animationIndex = {};
    // physical properties
    glm::vec3      dimensions = glm::vec3(0.0f);
    float          radius     = 0.0f;
};

#endif // MODEL_COMPONENT_DEFINE_HPP
