#include "player_system.hpp"
#include "../map/map_system.hpp"
#include "../entity_system/entity_system.hpp"
#include "../physics_system/physics_system.hpp"
#include <cmath>
#include <algorithm>

bool cPlayerSystem::initialize(void)
{
    m_players.clear();
    m_entityToIndex.clear();
    m_npcIndices.clear();
    return true;
}

void cPlayerSystem::terminate(void)
{
    for (sPlayerEvent* ev = getEvent(); ev != nullptr; ev = getEvent())
        delete ev;
    m_players.clear();
    m_entityToIndex.clear();
    m_npcIndices.clear();
}

void cPlayerSystem::addPlayer(uint32_t _playerIndex)
{
    // Check if already registered
    if (m_entityToIndex.find(_playerIndex) != m_entityToIndex.end())
        return;

    if (!m_entitySystem) return;

    sPlayer newPlayer;
    newPlayer.entityIndex   = _playerIndex;
    newPlayer.physicsIndex  = m_entitySystem->getPhysicsIndex(_playerIndex);
    newPlayer.graphicsIndex = m_entitySystem->getGraphicsIndex(_playerIndex);
    newPlayer.active        = true;

    // Determine starting tile from current physics position
    if (m_physicsSystem && newPlayer.physicsIndex != -1)
    {
        glm::vec3 pos = m_physicsSystem->getPosition(newPlayer.physicsIndex);
        newPlayer.currentTile = worldToTile(pos);
        newPlayer.targetTile  = newPlayer.currentTile;
        newPlayer.targetPosition = pos;
    }

    uint32_t index = static_cast<uint32_t>(m_players.size());
    m_players.push_back(newPlayer);
    m_entityToIndex[_playerIndex] = index;
}

void cPlayerSystem::removePlayer(uint32_t _playerIndex)
{
    auto it = m_entityToIndex.find(_playerIndex);
    if (it == m_entityToIndex.end())
        return;

    uint32_t index = it->second;
    sPlayer& player = m_players[index];
    player.active = false;

    // Optionally compact the vector later, but for simplicity we keep the slot.
    m_entityToIndex.erase(it);
}

void cPlayerSystem::setPlayerStartTile(uint32_t _playerIndex, uint32_t _tileIndex)
{
    // Find the player record
    auto it = m_entityToIndex.find(_playerIndex);
    if (it == m_entityToIndex.end())
        return;

    sPlayer& player = m_players[it->second];
    if (!player.active)
        return;

    // Validate tile index
    if (!m_mapSystem || _tileIndex >= m_mapSystem->getMap()->numTiles)
        return;

    // Convert tile index to world position (center of tile)
    glm::uvec2 dim = m_mapSystem->getDimensions();
    uint32_t tx = _tileIndex % dim.x;
    uint32_t tz = _tileIndex / dim.x;
    glm::vec3 spawnPos(tx + 0.5f, 0.0f, tz + 0.5f);

    // Update physics state (if available)
    if (m_physicsSystem && player.physicsIndex != -1)
    {
        m_physicsSystem->setPosition(player.physicsIndex, spawnPos);
        m_physicsSystem->setVelocity(player.physicsIndex, glm::vec3(0.0f));
        m_physicsSystem->setAcceleration(player.physicsIndex, glm::vec3(0.0f));
    }

    // Update graphics transform (if available)
    if (m_graphicsSystem && player.graphicsIndex != -1)
    {
        glm::vec3 dir = (m_physicsSystem && player.physicsIndex != -1)
                        ? m_physicsSystem->getDirection(player.physicsIndex)
                        : glm::vec3(0.0f, 0.0f, 1.0f);
        m_graphicsSystem->updateComponentMatrix(player.graphicsIndex, spawnPos, dir);
    }

    // Update player's internal state
    player.currentTile = _tileIndex;
    player.lastTile = _tileIndex;
    player.targetTile = _tileIndex;
    player.targetPosition = spawnPos;
    player.path.clear();
    player.pathIndex = 0;
}

// NPC index storage
void cPlayerSystem::addNPC(const uint32_t &_npcIndex)// register an existing NPC entity
{
    // Check if already registered
    if (m_entityToIndex.find(_npcIndex) != m_entityToIndex.end())
        return;

    if (!m_entitySystem) return;

    uint32_t index = static_cast<uint32_t>(m_npcIndices.size());
    m_npcIndices.push_back(_npcIndex);
    m_entityToIndex[_npcIndex] = index;
}

void cPlayerSystem::removeNPC(uint32_t _npcIndex)   // unregister an NPC entity
{
    auto it = m_entityToIndex.find(_npcIndex);
    if (it == m_entityToIndex.end())
        return;

    m_entityToIndex.erase(it);
}

void cPlayerSystem::setPlayerTarget(uint32_t _playerIndex, uint32_t targetTile)
{
    auto it = m_entityToIndex.find(_playerIndex);
    if (it == m_entityToIndex.end() || !m_mapSystem)
        return;

    sPlayer& p = m_players[it->second];
    if (!p.active || p.physicsIndex == -1)
        return;

    if (targetTile == p.currentTile)
        return; // already there

    std::vector<uint32_t> newPath;
    if (m_mapSystem->findPath(p.currentTile, targetTile, newPath))
    {
        onPlayerPathChanged(p, newPath);
    }
    else
    {
        // No path found – stop movement
        p.path.clear();
        p.pathIndex = 0;
        p.targetTile = p.currentTile;
        p.targetPosition = m_physicsSystem->getPosition(p.physicsIndex);
        if (m_physicsSystem)
        {
            m_physicsSystem->setVelocity(p.physicsIndex, glm::vec3(0.0f));
            m_physicsSystem->setAcceleration(p.physicsIndex, glm::vec3(0.0f));
        }
    }
}

void cPlayerSystem::setPlayerPosition(uint32_t _playerIndex, const glm::vec3& pos)
{
    auto it = m_entityToIndex.find(_playerIndex);
    if (it == m_entityToIndex.end())
        return;

    sPlayer& p = m_players[it->second];
    if (!p.active || p.physicsIndex == -1 || !m_physicsSystem)
        return;

    m_physicsSystem->setPosition(p.physicsIndex, pos);
    p.currentTile = worldToTile(pos);
    p.targetTile  = p.currentTile;
    p.targetPosition = pos;
    p.path.clear();
    p.pathIndex = 0;

    // Stop movement
    m_physicsSystem->setVelocity(p.physicsIndex, glm::vec3(0.0f));
    m_physicsSystem->setAcceleration(p.physicsIndex, glm::vec3(0.0f));
}

glm::vec3 cPlayerSystem::getPosition(uint32_t _playerIndex) const
{
    auto it = m_entityToIndex.find(_playerIndex);
    if (it == m_entityToIndex.end()) return glm::vec3(0);
    const sPlayer& p = m_players[it->second];
    if (p.physicsIndex == -1 || !m_physicsSystem) return glm::vec3(0);
    return m_physicsSystem->getPosition(p.physicsIndex);
}

glm::vec3 cPlayerSystem::getDirection(uint32_t _playerIndex) const
{
    auto it = m_entityToIndex.find(_playerIndex);
    if (it == m_entityToIndex.end()) return glm::vec3(0,0,1);
    const sPlayer& p = m_players[it->second];
    if (p.physicsIndex == -1 || !m_physicsSystem) return glm::vec3(0,0,1);
    return m_physicsSystem->getDirection(p.physicsIndex);
}

uint32_t cPlayerSystem::getCurrentTile(uint32_t _playerIndex) const
{
    auto it = m_entityToIndex.find(_playerIndex);
    if (it == m_entityToIndex.end()) return UINT32_MAX;
    return m_players[it->second].currentTile;
}

const std::vector<uint32_t>& cPlayerSystem::getPath(uint32_t _playerIndex) const
{
    static std::vector<uint32_t> empty;
    auto it = m_entityToIndex.find(_playerIndex);
    if (it == m_entityToIndex.end()) return empty;
    return m_players[it->second].path;
}

void cPlayerSystem::lookAtPlayer(uint32_t _playerIndex)
{
    if (!m_graphicsSystem) return;
    auto it = m_entityToIndex.find(_playerIndex);
    if (it == m_entityToIndex.end()) return;

    const sPlayer& p = m_players[it->second];
    if (p.physicsIndex == -1 || !m_physicsSystem) return;

    glm::vec3 playerPos = m_physicsSystem->getPosition(p.physicsIndex);
    glm::vec3 currentPos    = m_graphicsSystem->getCameraPosition();
    glm::vec3 currentLookAt = m_graphicsSystem->getCameraLookAt();
    glm::vec3 offset = currentPos - currentLookAt;
    glm::vec3 newPos = playerPos + offset;
    m_graphicsSystem->setCameraPosition(newPos, playerPos);
}

// ----------------------------------------------------------------------------
// Internal Helpers
// ----------------------------------------------------------------------------

uint32_t cPlayerSystem::worldToTile(const glm::vec3& pos) const
{
    if (!m_mapSystem) return 0;
    glm::uvec2 dim = m_mapSystem->getDimensions();
    uint32_t x = static_cast<uint32_t>(glm::clamp(static_cast<int>(pos.x), 0, static_cast<int>(dim.x)-1));
    uint32_t z = static_cast<uint32_t>(glm::clamp(static_cast<int>(pos.z), 0, static_cast<int>(dim.y)-1));
    return x + z * dim.x;
}

glm::vec3 cPlayerSystem::tileToWorld(uint32_t tile) const
{
    if (!m_mapSystem) return glm::vec3(0);
    glm::uvec2 dim = m_mapSystem->getDimensions();
    uint32_t x = tile % dim.x;
    uint32_t z = tile / dim.x;
    return glm::vec3(x + 0.5f, 0.0f, z + 0.5f);
}

void cPlayerSystem::recomputeTargetPosition(sPlayer& player)
{
    player.targetPosition = tileToWorld(player.targetTile);
}

void cPlayerSystem::onPlayerPathChanged(sPlayer& player, const std::vector<uint32_t>& newPath)
{
    if (newPath.size() < 2)
    {
        player.path.clear();
        player.pathIndex = 0;
        player.waypoints.clear();
        player.waypointIndex = 0;
        player.targetTile = player.currentTile;
        player.targetPosition = m_physicsSystem->getPosition(player.physicsIndex);
        m_physicsSystem->setVelocity(player.physicsIndex, glm::vec3(0.0f));
        m_physicsSystem->setAcceleration(player.physicsIndex, glm::vec3(0.0f));
        return;
    }

    player.path = newPath;
    player.pathIndex = 1; // start from the second tile
    player.targetTile = player.path[player.pathIndex];
    recomputeTargetPosition(player);

    // Generate waypoints: only keep points where direction changes (corners)
    player.waypoints.clear();
    glm::uvec2 dim = m_mapSystem->getDimensions();

    auto tileToWorld = [&](uint32_t tile) -> glm::vec3 {
        uint32_t x = tile % dim.x;
        uint32_t z = tile / dim.x;
        return glm::vec3(x + 0.5f, 0.0f, z + 0.5f);
    };

    // Always include the first tile's centre
    player.waypoints.push_back(tileToWorld(newPath.front()));

    // Add only direction-change points
    for (size_t i = 1; i < newPath.size() - 1; ++i)
    {
        uint32_t prev = newPath[i-1];
        uint32_t curr = newPath[i];
        uint32_t next = newPath[i+1];

        int dx1 = static_cast<int>(curr % dim.x) - static_cast<int>(prev % dim.x);
        int dz1 = static_cast<int>(curr / dim.x) - static_cast<int>(prev / dim.x);
        int dx2 = static_cast<int>(next % dim.x) - static_cast<int>(curr % dim.x);
        int dz2 = static_cast<int>(next / dim.x) - static_cast<int>(curr / dim.x);

        if (dx1 != dx2 || dz1 != dz2) // direction changed
            player.waypoints.push_back(tileToWorld(curr));
    }

    // Always include the last tile's centre
    player.waypoints.push_back(tileToWorld(newPath.back()));

    player.waypointIndex = 1; // start moving toward the second waypoint
}

// ----------------------------------------------------------------------------
// Process
// ----------------------------------------------------------------------------

void cPlayerSystem::process(float deltaTime)
{
    if (!m_physicsSystem || !m_mapSystem) return;

    for (auto& player : m_players)
    {
        if (!player.active) continue;
        if (player.physicsIndex == -1) continue;

        // Skip if no waypoints remain
        if (player.waypointIndex >= player.waypoints.size())
        {
            m_physicsSystem->setVelocity(player.physicsIndex, glm::vec3(0.0f));
            m_physicsSystem->setAcceleration(player.physicsIndex, glm::vec3(0.0f));
            continue;
        }

        glm::vec3 target = player.waypoints[player.waypointIndex];
        glm::vec3 currentPos = m_physicsSystem->getPosition(player.physicsIndex);
        glm::vec3 toTarget = target - currentPos;
        float distance = glm::length(toTarget);

        // Arrival threshold – when close enough, snap to the waypoint and advance
        const float arrivalThreshold = 0.1f;
        if (distance < arrivalThreshold)
        {
            // Snap to exact centre
            m_physicsSystem->setPosition(player.physicsIndex, target);
            m_physicsSystem->setVelocity(player.physicsIndex, glm::vec3(0.0f));
            m_physicsSystem->setAcceleration(player.physicsIndex, glm::vec3(0.0f));

            player.waypointIndex++;
            continue;
        }

        // Determine desired speed based on distance to next turn
        float maxSpeed = m_physicsSystem->getMaxVelocity(player.physicsIndex).x;
        float desiredSpeed = maxSpeed;

        // Look ahead: if the next waypoint is a turn, start slowing early
        if (player.waypointIndex + 1 < player.waypoints.size())
        {
            glm::vec3 nextTarget = player.waypoints[player.waypointIndex + 1];
            glm::vec3 dirToCurrent = glm::normalize(toTarget);
            glm::vec3 dirToNext = glm::normalize(nextTarget - target);
            float dot = glm::dot(dirToCurrent, dirToNext);

            // If the angle is sharp (>45°), reduce speed before reaching the turn
            const float turnThreshold = 0.7f; // cos(45°)
            if (dot < turnThreshold)
            {
                float distToTurn = distance; // distance to the corner
                float slowingRadius = 1.5f;   // start slowing 1.5 units before the corner
                if (distToTurn < slowingRadius)
                    desiredSpeed = maxSpeed * (distToTurn / slowingRadius);
            }
        }

        // Standard arrival behaviour for the current waypoint
        const float slowingRadius = 1.0f;
        if (distance < slowingRadius)
            desiredSpeed *= (distance / slowingRadius);

        desiredSpeed = glm::max(desiredSpeed, 0.5f); // keep a minimum speed

        glm::vec3 desiredDir = toTarget / distance;
        glm::vec3 desiredVel = desiredDir * desiredSpeed;
        glm::vec3 currentVel = m_physicsSystem->getVelocity(player.physicsIndex);
        glm::vec3 steering = desiredVel - currentVel;

        // Apply acceleration limits
        glm::vec3 maxAccel = m_physicsSystem->getMaxAcceleration(player.physicsIndex);
        float accelMag = glm::length(steering);
        if (accelMag > maxAccel.x)
            steering = steering / accelMag * maxAccel.x;

        m_physicsSystem->setAcceleration(player.physicsIndex, steering);

        // Tile change detection (unchanged)
        uint32_t newTile = worldToTile(currentPos);
        if (newTile != player.lastTile)
        {
            player.lastTile = newTile;
            player.currentTile = newTile;
            sPlayerEvent* ev = new sPlayerEvent;
            ev->type = ePlayerEventType::tileChange;
            ev->data = newTile;
            m_event.push(ev);
        }

        sPlayerEvent* ev = new sPlayerEvent;
        ev->type = ePlayerEventType::positionChange;
        ev->data = player.entityIndex;
        m_event.push(ev);
    }
}
