#include "npc_system.hpp"
#include "../entity_system/entity_system.hpp"
#include "../map/map_system.hpp"
#include "../physics_system/physics_system.hpp"
#include "../player/player_system.hpp"
#include <glm/gtx/norm.hpp>
#include <algorithm>
#include <limits>

bool cNPCSystem::initialize()
{
    m_NPCs.clear();
    m_entityToIndex.clear();
    m_playerIndices.clear();
    return true;
}

void cNPCSystem::terminate()
{
    for (sNPCEvent* ev = getEvent(); ev != nullptr; ev = getEvent())
        delete ev;
    m_NPCs.clear();
    m_entityToIndex.clear();
    m_playerIndices.clear();
}

void cNPCSystem::addNPC(uint32_t _npcIndex)
{
    if (m_entityToIndex.find(_npcIndex) != m_entityToIndex.end())
        return;
    if (!m_entitySystem) return;

    sNPC newNPC;
    newNPC.entityIndex   = _npcIndex;
    newNPC.physicsIndex  = m_entitySystem->getPhysicsIndex(_npcIndex);
    newNPC.graphicsIndex = m_entitySystem->getGraphicsIndex(_npcIndex);
    newNPC.active        = true;

    if (m_physicsSystem && newNPC.physicsIndex != -1)
    {
        glm::vec3 pos = m_physicsSystem->getPosition(newNPC.physicsIndex);
        newNPC.currentTile = worldToTile(pos);
        newNPC.targetTile  = newNPC.currentTile;
        newNPC.targetPosition = pos;
    }

    uint32_t index = static_cast<uint32_t>(m_NPCs.size());
    m_NPCs.push_back(newNPC);
    m_entityToIndex[_npcIndex] = index;
}

void cNPCSystem::removeNPC(uint32_t _npcIndex)
{
    auto it = m_entityToIndex.find(_npcIndex);
    if (it == m_entityToIndex.end()) return;
    uint32_t index = it->second;
    m_NPCs[index].active = false;
    m_entityToIndex.erase(it);
}

void cNPCSystem::addPlayer(const uint32_t& _playerIndex)
{
    m_playerIndices.push_back(_playerIndex);
}


uint32_t cNPCSystem::worldToTile(const glm::vec3& pos) const
{
    if (!m_mapSystem) return 0;
    glm::uvec2 dim = m_mapSystem->getDimensions();
    uint32_t x = static_cast<uint32_t>(glm::clamp(static_cast<int>(pos.x), 0, static_cast<int>(dim.x)-1));
    uint32_t z = static_cast<uint32_t>(glm::clamp(static_cast<int>(pos.z), 0, static_cast<int>(dim.y)-1));
    return x + z * dim.x;
}

glm::vec3 cNPCSystem::tileToWorld(uint32_t tile) const
{
    if (!m_mapSystem) return glm::vec3(0);
    glm::uvec2 dim = m_mapSystem->getDimensions();
    uint32_t x = tile % dim.x;
    uint32_t z = tile / dim.x;
    return glm::vec3(x + 0.5f, 0.0f, z + 0.5f);
}

void cNPCSystem::onNPCPathChanged(sNPC& npc, const std::vector<uint32_t>& newPath)
{
    if (newPath.size() < 2)
    {
        npc.path.clear();
        npc.pathIndex = 0;
        npc.waypoints.clear();
        npc.waypointIndex = 0;
        npc.targetTile = npc.currentTile;
        npc.targetPosition = m_physicsSystem->getPosition(npc.physicsIndex);
        m_physicsSystem->setVelocity(npc.physicsIndex, glm::vec3(0.0f));
        m_physicsSystem->setAcceleration(npc.physicsIndex, glm::vec3(0.0f));
        return;
    }

    npc.path = newPath;
    npc.pathIndex = 1;
    npc.targetTile = npc.path[npc.pathIndex];
    npc.targetPosition = tileToWorld(npc.targetTile);

    // Generate waypoints: only keep points where direction changes
    npc.waypoints.clear();
    glm::uvec2 dim = m_mapSystem->getDimensions();

    auto tileToWorldLambda = [&](uint32_t tile) -> glm::vec3 {
        uint32_t x = tile % dim.x;
        uint32_t z = tile / dim.x;
        return glm::vec3(x + 0.5f, 0.0f, z + 0.5f);
    };

    npc.waypoints.push_back(tileToWorldLambda(newPath.front()));

    for (size_t i = 1; i < newPath.size() - 1; ++i)
    {
        uint32_t prev = newPath[i-1];
        uint32_t curr = newPath[i];
        uint32_t next = newPath[i+1];

        int dx1 = static_cast<int>(curr % dim.x) - static_cast<int>(prev % dim.x);
        int dz1 = static_cast<int>(curr / dim.x) - static_cast<int>(prev / dim.x);
        int dx2 = static_cast<int>(next % dim.x) - static_cast<int>(curr % dim.x);
        int dz2 = static_cast<int>(next / dim.x) - static_cast<int>(curr / dim.x);

        if (dx1 != dx2 || dz1 != dz2)
            npc.waypoints.push_back(tileToWorldLambda(curr));
    }

    npc.waypoints.push_back(tileToWorldLambda(newPath.back()));
    npc.waypointIndex = 1; // start moving toward second waypoint
}

void cNPCSystem::process(float deltaTime)
{
    if (m_NPCs.empty() || !m_playerSystem) return;
    if (!m_physicsSystem || !m_mapSystem) return;

    for (auto& npc : m_NPCs)
    {
        if (!npc.active) continue;
        if (npc.physicsIndex == -1) continue;

        // Find closest player
        float bestDistSq = std::numeric_limits<float>::max();
        uint32_t bestPlayerIdx = 0;
        const glm::vec3& npcPos = m_physicsSystem->getPosition(npc.physicsIndex);

        for (uint32_t playerIdx : m_playerIndices)   // m_playerIndices must be filled externally
        {
            int32_t playerPhys = m_entitySystem->getPhysicsIndex(playerIdx);
            if (playerPhys < 0) continue;
            const glm::vec3& playerPos = m_physicsSystem->getPosition(playerPhys);
            float distSq = glm::distance2(npcPos, playerPos);
            if (distSq < bestDistSq)
            {
                bestDistSq = distSq;
                bestPlayerIdx = playerIdx;
            }
        }

        if (bestDistSq == std::numeric_limits<float>::max())
            continue; // no valid player

        // Get player's tile
        int32_t playerPhys = m_entitySystem->getPhysicsIndex(bestPlayerIdx);
        if (playerPhys < 0) continue;
        const glm::vec3& playerPos = m_physicsSystem->getPosition(playerPhys);
        uint32_t goalTile = worldToTile(playerPos);

        // Recompute path if target changed or no waypoints
        if (goalTile != npc.targetTile || npc.waypoints.empty())
        {
            uint32_t startTile = worldToTile(npcPos);
            std::vector<uint32_t> tilePath;
            if (m_mapSystem->findPath(startTile, goalTile, tilePath))
                onNPCPathChanged(npc, tilePath);
            else
            {
                // No path – stop moving
                npc.waypoints.clear();
                npc.waypointIndex = 0;
                npc.targetTile = startTile;
                m_physicsSystem->setVelocity(npc.physicsIndex, glm::vec3(0.0f));
                m_physicsSystem->setAcceleration(npc.physicsIndex, glm::vec3(0.0f));
                continue;
            }
        }

        // Follow waypoints using steering (same as player system)
        if (npc.waypointIndex >= npc.waypoints.size())
        {
            m_physicsSystem->setVelocity(npc.physicsIndex, glm::vec3(0.0f));
            m_physicsSystem->setAcceleration(npc.physicsIndex, glm::vec3(0.0f));
            continue;
        }

        glm::vec3 target = npc.waypoints[npc.waypointIndex];
        glm::vec3 currentPos = m_physicsSystem->getPosition(npc.physicsIndex);
        glm::vec3 toTarget = target - currentPos;
        float distance = glm::length(toTarget);

        const float arrivalThreshold = 0.1f;
        if (distance < arrivalThreshold)
        {
            m_physicsSystem->setPosition(npc.physicsIndex, target);
            m_physicsSystem->setVelocity(npc.physicsIndex, glm::vec3(0.0f));
            m_physicsSystem->setAcceleration(npc.physicsIndex, glm::vec3(0.0f));
            npc.waypointIndex++;
            continue;
        }

        float maxSpeed = m_physicsSystem->getMaxVelocity(npc.physicsIndex).x;
        if (maxSpeed <= 0.0f) maxSpeed = 5.0f;
        float desiredSpeed = maxSpeed;

        // Slow down before turns
        if (npc.waypointIndex + 1 < npc.waypoints.size())
        {
            glm::vec3 nextTarget = npc.waypoints[npc.waypointIndex + 1];
            glm::vec3 dirToCurrent = glm::normalize(toTarget);
            glm::vec3 dirToNext = glm::normalize(nextTarget - target);
            float dot = glm::dot(dirToCurrent, dirToNext);
            const float turnThreshold = 0.7f; // cos(45°)
            if (dot < turnThreshold)
            {
                float distToTurn = distance;
                float slowingRadius = 1.5f;
                if (distToTurn < slowingRadius)
                    desiredSpeed = maxSpeed * (distToTurn / slowingRadius);
            }
        }

        const float slowingRadius = 1.0f;
        if (distance < slowingRadius)
            desiredSpeed *= (distance / slowingRadius);
        desiredSpeed = glm::max(desiredSpeed, 0.5f);

        glm::vec3 desiredDir = toTarget / distance;
        glm::vec3 desiredVel = desiredDir * desiredSpeed;
        glm::vec3 currentVel = m_physicsSystem->getVelocity(npc.physicsIndex);
        glm::vec3 steering = desiredVel - currentVel;

        glm::vec3 maxAccel = m_physicsSystem->getMaxAcceleration(npc.physicsIndex);
        float accelMag = glm::length(steering);
        if (accelMag > maxAccel.x)
            steering = steering / accelMag * maxAccel.x;

        m_physicsSystem->setAcceleration(npc.physicsIndex, steering);

        // Tile change detection and events
        uint32_t newTile = worldToTile(currentPos);
        if (newTile != npc.lastTile)
        {
            npc.lastTile = newTile;
            npc.currentTile = newTile;
            // Optionally emit an NPC tile change event (not defined, but can be added)
        }

        sNPCEvent* evt = new sNPCEvent;
        evt->type = eNPCEventType::positionChange;
        evt->dataEntity = npc.entityIndex;
        evt->dataPhysics = npc.physicsIndex;
        m_event.push(evt);
    }
}
