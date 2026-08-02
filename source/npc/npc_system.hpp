
#ifndef NPC_SYSTEM_HPP
#define NPC_SYSTEM_HPP

#include <glm/glm.hpp>
#include <cstdint>
#include <vector>
#include <unordered_map>

#include "../template/template_queue.hpp"
#include "npc_define.hpp"
#include "npc_event_define.hpp"

class cEntitySystem;   // forward declaration
class cMapSystem;      // forward declaration
class cPhysicsSystem;  // forward declaration
class cPlayerSystem;   // forward declaration

class cNPCSystem
{
    public:
        bool initialize();
        void terminate();
        void process(float deltaTime);

        void addNPC(uint32_t _npcIndex);      // register an existing NPC entity
        void removeNPC(uint32_t _npcIndex);   // unregister an NPC entity
        void addPlayer(const uint32_t &_playerIndex);

        void setEntityPointer(cEntitySystem* _entity)   { m_entitySystem = _entity; }
        void setMapPointer(cMapSystem* _map)            { m_mapSystem = _map; }
        void setPhysicsPointer(cPhysicsSystem* _physics){ m_physicsSystem = _physics; }
        void setPlayerSystem(cPlayerSystem* _player)    { m_playerSystem = _player; }

        sNPCEvent* getEvent(void) { return m_event.pop(); }

    private:
        // Event queue
        tcQueue<sNPCEvent> m_event = {};

        // External systems
        cEntitySystem*  m_entitySystem  = nullptr;
        cMapSystem*     m_mapSystem     = nullptr;
        cPhysicsSystem* m_physicsSystem = nullptr;
        cPlayerSystem*  m_playerSystem  = nullptr;

        // NPC storage (indexed by entity index, stored in vector for iteration)
        std::vector<sNPC> m_NPCs;
        std::unordered_map<uint32_t, uint32_t> m_entityToIndex; // entityId -> index in m_NPCs

        // Player indices storage
        std::vector<uint32_t> m_playerIndices; // one per Player

        // Internal helpers (similar to player system)
        uint32_t worldToTile(const glm::vec3& pos) const;
        glm::vec3 tileToWorld(uint32_t tile) const;
        void onNPCPathChanged(sNPC& npc, const std::vector<uint32_t>& newPath);
};

#endif // NPC_SYSTEM_HPP

