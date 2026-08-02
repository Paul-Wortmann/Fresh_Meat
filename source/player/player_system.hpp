#ifndef PLAYER_SYSTEM_HPP
#define PLAYER_SYSTEM_HPP

#include <glm/glm.hpp>
#include <cstdint>
#include <vector>
#include <unordered_map>

#include "../template/template_queue.hpp"
#include "player_define.hpp"
#include "player_event_define.hpp"

class cEntitySystem;
class cGraphicsSystem;
class cMapSystem;
class cPhysicsSystem;

class cPlayerSystem
{
public:
    bool initialize(void);
    void terminate(void);
    void process(float deltaTime);

    // External system injection
    void setGraphicsPointer(cGraphicsSystem* graphics) { m_graphicsSystem = graphics; }
    void setMapPointer(cMapSystem* map)               { m_mapSystem = map; }
    void setEntityPointer(cEntitySystem* entity)      { m_entitySystem = entity; }
    void setPhysicsPointer(cPhysicsSystem* physics)   { m_physicsSystem = physics; }

    // Player management
    void addPlayer(uint32_t _playerIndex);      // register an existing player entity
    void removePlayer(uint32_t _playerIndex);   // unregister a player entity
    void setPlayerStartTile(uint32_t _playerIndex, uint32_t _tileIndex);

    // NPC index storage
    void addNPC(const uint32_t &_npcIndex);// register an existing NPC entity
    void removeNPC(uint32_t _npcIndex);   // unregister an NPC entity

    // Commands
    void setPlayerTarget(uint32_t _playerIndex, uint32_t targetTile);
    void setPlayerPosition(uint32_t _playerIndex, const glm::vec3& pos); // manual override (teleport)

    // Queries
    glm::vec3 getPosition(uint32_t _playerIndex) const;
    glm::vec3 getDirection(uint32_t _playerIndex) const;
    uint32_t  getCurrentTile(uint32_t _playerIndex) const;
    const std::vector<uint32_t>& getPath(uint32_t _playerIndex) const;

    // Camera helpers
    void lookAtPlayer(uint32_t _playerIndex);

    // Event interface
    sPlayerEvent* getEvent(void) { return m_event.pop(); }

private:
    // External system pointers
    cEntitySystem*   m_entitySystem   = nullptr;
    cGraphicsSystem* m_graphicsSystem = nullptr;
    cMapSystem*      m_mapSystem      = nullptr;
    cPhysicsSystem*  m_physicsSystem  = nullptr;

    // Player storage (indexed by entity index, but stored in vector for iteration)
    std::vector<sPlayer> m_players;
    std::unordered_map<uint32_t, uint32_t> m_entityToIndex; // entityId -> index in m_players

    // NPC indices storage (we will use this later for player-entity interaction)
    std::vector<uint32_t> m_npcIndices; // one per NPC

    // Movement parameters
    float m_stopDistance = 0.001f;   // distance threshold to consider tile reached

    // Event queue
    tcQueue<sPlayerEvent> m_event;

    // Internal helpers
    void onPlayerPathChanged(sPlayer& player, const std::vector<uint32_t>& newPath);
    void recomputeTargetPosition(sPlayer& player);
    uint32_t worldToTile(const glm::vec3& pos) const;
    glm::vec3 tileToWorld(uint32_t tile) const;
};

#endif // PLAYER_SYSTEM_HPP
