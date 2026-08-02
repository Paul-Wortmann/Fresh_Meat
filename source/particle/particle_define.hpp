#ifndef PARTICLE_DEFINE_HPP
#define PARTICLE_DEFINE_HPP

#include <glm/glm.hpp>
#include <cstdint>

struct sParticle
{
    bool      enabled = false;
    float     life    = 0.0f;          // remaining lifetime (seconds)
    float     initialLife = 0.0f;      // total lifetime (for interpolation)

    glm::vec3 position     = {};
    glm::vec3 velocity     = {};
    glm::vec3 acceleration = {};

    glm::vec4 color        = {1.0f, 1.0f, 1.0f, 1.0f};
    float     size         = 1.0f;

    std::size_t emitterId = 0;          // which emitter owns this particle
};

struct sParticleEmitter
{
    // Emitter parameters
    glm::vec3 position     = {0.0f, 0.0f, 0.0f};
    float     spread       = 1.0f;                // cone spread (radians)
    float     minLife      = 1.0f;
    float     maxLife      = 3.0f;
    float     minSpeed     = 1.0f;
    float     maxSpeed     = 5.0f;
    glm::vec3 gravity      = {0.0f, -9.81f, 0.0f};

    // Visual interpolation
    glm::vec4 startColor   = {1.0f, 1.0f, 1.0f, 1.0f};
    glm::vec4 endColor     = {1.0f, 1.0f, 1.0f, 0.0f};
    float     startSize    = 1.0f;
    float     endSize      = 0.0f;

    // Spawn rate (particles per second) – optional for continuous emission
    float     spawnRate    = 10.0f;
    float     spawnAccum   = 0.0f;

    bool      enabled      = true;
};

#endif
