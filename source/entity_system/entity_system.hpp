#ifndef ENTITY_SYSTEM_HPP
#define ENTITY_SYSTEM_HPP

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

#include "entity_define.hpp"
#include "entity_event_define.hpp"
#include "../template/template_queue.hpp"
#include "../core/defines.hpp"

class cAudioSystem;    // forward declaration
class cGraphicsSystem; // forward declaration
class cPhysicsSystem;  // forward declaration

class cEntitySystem
{
    public:
        bool initialize(void);
        void terminate(void);
        void process(float _delta);

        std::uint32_t getNewEntity(void);
        void          destroyEntity(const std::int32_t &_index);
        std::int32_t  loadEntity(const std::string &_fileName, const glm::vec3 &_position = glm::vec3(0));

        void setAudioSystem(cAudioSystem *_audioSystem)    { m_audioSystem = _audioSystem; }
        void setGraphicsSystem(cGraphicsSystem *_graphicsSystem) { m_graphicsSystem = _graphicsSystem; }
        void setPhysicsSystem(cPhysicsSystem *_physicsSystem) { m_physicsSystem = _physicsSystem; }

        std::int32_t getPlayerIndex(void) const { return m_playerIndex; }
        std::int32_t getPhysicsIndex(void) const { return (m_playerIndex >= 0) ? m_entity[m_playerIndex].physicsComponent : -1; }
        std::int32_t getPhysicsIndex(const std::int32_t &_entityIndex) const { return (m_playerIndex >= 0) ? m_entity[_entityIndex].physicsComponent : -1; }
        std::int32_t getGraphicsIndex(void) const { return (m_playerIndex >= 0) ? m_entity[m_playerIndex].graphicsComponent : -1; }
        std::int32_t getGraphicsIndex(const std::int32_t &_entityIndex) const { return (m_playerIndex >= 0) ? m_entity[_entityIndex].graphicsComponent : -1; }

        bool hasAudio(std::uint32_t _entity) const;
        bool hasGraphics(std::uint32_t _entity) const;
        bool hasPhysics(std::uint32_t _entity) const;

        std::int32_t getEntityIDFromPhysicsID(const std::int32_t &_physicsIndex) const;

        // event interface
        sEntityEvent* getEvent(void) { return m_event.pop(); }

    private:
        // event
        tcQueue<sEntityEvent> m_event = {};

        std::vector<sEntity> m_entity;
        std::vector<std::uint32_t> m_freeList;
        std::unordered_map<std::int32_t, std::int32_t> m_physicsToEntity;

        std::int32_t m_playerIndex = -1;

        cAudioSystem*    m_audioSystem    = nullptr;
        cGraphicsSystem* m_graphicsSystem = nullptr;
        cPhysicsSystem*  m_physicsSystem  = nullptr;
};

#endif // ENTITY_SYSTEM_HPP
