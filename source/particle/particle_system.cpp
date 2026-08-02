/*

Example usage:

cParticleSystem ps;
ps.initialize();

sParticleEmitter fireEmitter;
fireEmitter.position = {0, 0, 0};
fireEmitter.spread = 0.5f;
fireEmitter.minLife = 0.5f;
fireEmitter.maxLife = 1.2f;
fireEmitter.minSpeed = 2.0f;
fireEmitter.maxSpeed = 5.0f;
fireEmitter.startColor = {1, 0.5, 0, 1};
fireEmitter.endColor = {1, 0, 0, 0};
fireEmitter.startSize = 0.5f;
fireEmitter.endSize = 0.0f;
fireEmitter.spawnRate = 30.0f;  // 30 particles per second

std::size_t fireId = ps.createEmitter(fireEmitter);

// In your game loop:
float delta = 0.016f;
ps.process(delta);   // automatically spawns particles based on spawnRate

// Or manually spawn a burst:
ps.spawnParticles(fireId, 100);

*/

#include "particle_system.hpp"
#include <glm/gtc/constants.hpp>
#include <glm/gtc/random.hpp>
#include <algorithm>
#include <cassert>

bool cParticleSystem::initialize()
{
    m_particles.clear();
    m_particleFreeList.clear();
    m_emitters.clear();
    m_emitterFreeList.clear();
    return true;
}

void cParticleSystem::terminate()
{
    m_particles.clear();
    m_particleFreeList.clear();
    m_emitters.clear();
    m_emitterFreeList.clear();
}

void cParticleSystem::process(float delta)
{
    // 1. Update existing particles
    for (auto& p : m_particles)
    {
        if (!p.enabled)
            continue;

        // Decrease lifetime
        p.life -= delta;
        if (p.life <= 0.0f)
        {
            p.enabled = false;
            continue;
        }

        // Retrieve emitter properties
        assert(p.emitterId < m_emitters.size() && "Invalid emitter ID");
        const sParticleEmitter& e = m_emitters[p.emitterId];

        // Apply gravity (or any constant acceleration)
        p.acceleration = e.gravity;

        // Euler integration
        p.velocity += p.acceleration * delta;
        p.position += p.velocity * delta;

        // Interpolate color and size based on remaining life fraction
        float t = 1.0f - (p.life / p.initialLife);  // 0 = born, 1 = dead
        p.color = glm::mix(e.startColor, e.endColor, t);
        p.size  = glm::mix(e.startSize,  e.endSize,  t);
    }

    // 2. Rebuild particle freelist from those that died
    m_particleFreeList.clear();
    for (std::size_t i = 0; i < m_particles.size(); ++i)
    {
        if (!m_particles[i].enabled)
            m_particleFreeList.push_back(i);
    }

    // 3. Continuous emission (optional – you can call spawnParticles manually instead)
    for (std::size_t i = 0; i < m_emitters.size(); ++i)
    {
        if (!m_emitters[i].enabled)
            continue;

        sParticleEmitter& e = m_emitters[i];
        if (e.spawnRate > 0.0f)
        {
            e.spawnAccum += delta;
            float spawnInterval = 1.0f / e.spawnRate;
            while (e.spawnAccum >= spawnInterval)
            {
                spawnParticles(i, 1);   // spawn one particle per interval
                e.spawnAccum -= spawnInterval;
            }
        }
    }
}

void cParticleSystem::spawnParticles(std::size_t emitterIndex, std::uint32_t count)
{
    if (emitterIndex >= m_emitters.size())
        return;

    const sParticleEmitter& e = m_emitters[emitterIndex];

    std::uniform_real_distribution<float> lifeDist(e.minLife, e.maxLife);
    std::uniform_real_distribution<float> speedDist(e.minSpeed, e.maxSpeed);
    std::uniform_real_distribution<float> angleDist(-e.spread, e.spread);

    for (std::uint32_t i = 0; i < count; ++i)
    {
        std::size_t idx = getNewParticle();
        sParticle& p = m_particles[idx];

        p.initialLife = lifeDist(m_rng);
        p.life = p.initialLife;
        p.emitterId = emitterIndex;

        // Random direction within a cone around Y axis
        float yaw   = angleDist(m_rng);
        float pitch = angleDist(m_rng);
        glm::vec3 dir;
        dir.x = std::sin(yaw) * std::cos(pitch);
        dir.y = std::sin(pitch);
        dir.z = std::cos(yaw) * std::cos(pitch);
        dir = glm::normalize(dir);

        float speed = speedDist(m_rng);
        p.velocity = dir * speed;
        p.position = e.position;
        p.acceleration = e.gravity;

        // Initial color/size (will be interpolated in process)
        p.color = e.startColor;
        p.size  = e.startSize;
    }
}

// ----- Particle freelist -----
std::size_t cParticleSystem::getNewParticle()
{
    if (!m_particleFreeList.empty())
    {
        std::size_t idx = m_particleFreeList.back();
        m_particleFreeList.pop_back();
        m_particles[idx] = sParticle{};
        m_particles[idx].enabled = true;
        return idx;
    }

    m_particles.emplace_back();
    m_particles.back().enabled = true;
    return m_particles.size() - 1;
}

void cParticleSystem::destroyParticle(std::size_t index)
{
    if (index >= m_particles.size() || !m_particles[index].enabled)
        return;
    m_particles[index].enabled = false;
    m_particleFreeList.push_back(index);
}

// ----- Emitter management -----
std::size_t cParticleSystem::createEmitter(const sParticleEmitter& emitter)
{
    std::size_t idx = getNewEmitter();
    m_emitters[idx] = emitter;
    m_emitters[idx].enabled = true;
    return idx;
}

void cParticleSystem::destroyEmitter(std::size_t index)
{
    if (index >= m_emitters.size() || !m_emitters[index].enabled)
        return;

    // Also destroy all particles belonging to this emitter
    for (auto& p : m_particles)
    {
        if (p.enabled && p.emitterId == index)
            p.enabled = false;
    }
    // Rebuild particle freelist (could be optimised, but simple)
    m_particleFreeList.clear();
    for (std::size_t i = 0; i < m_particles.size(); ++i)
        if (!m_particles[i].enabled)
            m_particleFreeList.push_back(i);

    m_emitters[index].enabled = false;
    m_emitterFreeList.push_back(index);
}

sParticleEmitter& cParticleSystem::getEmitter(std::size_t index)
{
    assert(index < m_emitters.size() && "Invalid emitter index");
    return m_emitters[index];
}

std::size_t cParticleSystem::getEmitterCount() const
{
    return m_emitters.size() - m_emitterFreeList.size();
}

std::size_t cParticleSystem::getNewEmitter()
{
    if (!m_emitterFreeList.empty())
    {
        std::size_t idx = m_emitterFreeList.back();
        m_emitterFreeList.pop_back();
        m_emitters[idx] = sParticleEmitter{};
        m_emitters[idx].enabled = true;
        return idx;
    }

    m_emitters.emplace_back();
    m_emitters.back().enabled = true;
    return m_emitters.size() - 1;
}

std::size_t cParticleSystem::getActiveParticleCount() const
{
    return m_particles.size() - m_particleFreeList.size();
}

glm::vec3 cParticleSystem::getEmitterPosition(std::size_t index) const
{
    if (index >= m_emitters.size())
        return glm::vec3(0.0f);
    return m_emitters[index].position;
}

void cParticleSystem::setEmitterPosition(std::size_t index, const glm::vec3& pos)
{
    if (index >= m_emitters.size())
        return;
    m_emitters[index].position = pos;
}
