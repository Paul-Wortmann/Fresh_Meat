#ifndef PARTICLE_SYSTEM_HPP
#define PARTICLE_SYSTEM_HPP

#include <vector>
#include <random>
#include "particle_define.hpp"

class cParticleSystem
{
public:
    bool initialize();
    void terminate();
    void process(float delta);

    // Spawn particles from a specific emitter
    void spawnParticles(std::size_t emitterIndex, std::uint32_t count);

    // Emitter management
    [[nodiscard]] std::size_t createEmitter(const sParticleEmitter& emitter);
    void destroyEmitter(std::size_t index);
    [[nodiscard]] sParticleEmitter& getEmitter(std::size_t index);
    [[nodiscard]] std::size_t getEmitterCount() const;

    // Emitter position access
    [[nodiscard]] glm::vec3 getEmitterPosition(std::size_t index) const;
    void setEmitterPosition(std::size_t index, const glm::vec3& pos);

    // Particle query
    [[nodiscard]] std::size_t getActiveParticleCount() const;

    // Global Particle access
    std::vector<sParticle>& getParticles(void) { return m_particles; }
    std::vector<sParticleEmitter>& getParticleEmitters(void) { return m_emitters; }

private:
    // Particles
    std::vector<sParticle>        m_particles;
    std::vector<std::size_t>      m_particleFreeList;
    [[nodiscard]] std::size_t getNewParticle();
    void destroyParticle(std::size_t index);

    // Emitters
    std::vector<sParticleEmitter> m_emitters;
    std::vector<std::size_t>      m_emitterFreeList;
    [[nodiscard]] std::size_t getNewEmitter();

    // Random engine
    std::mt19937 m_rng{ std::random_device{}() };
};

#endif
