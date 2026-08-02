#ifndef PHYSICS_SYSTEM_HPP
#define PHYSICS_SYSTEM_HPP

#include <vector>
#include <cstdint>
#include <iostream>
#include <cassert>
#include "physics_component_define.hpp"
#include "physics_event_define.hpp"
#include "../template/template_queue.hpp"

class cEntitySystem;   // forward declaration
class cGraphicsSystem; // forward declaration

class cPhysicsSystem
{
    public:
        bool initialize(void);
        void terminate(void);
        void process(float _delta);

        // External systems
        void setEntitySystem(cEntitySystem *_entitySystem) { m_entitySystem = _entitySystem; }
        void setGraphicsSystem(cGraphicsSystem *_graphicsSystem) { m_graphicsSystem = _graphicsSystem; }

        // component management
        std::uint32_t getNewComponent(void);
        void          destroyComponent(const std::uint32_t& _index);

        // linear property getters
        const glm::vec3& getDirection(std::uint32_t _index) const;
        const glm::vec3& getPosition(std::uint32_t _index) const;
        const glm::vec3& getVelocity(std::uint32_t _index) const;
        const glm::vec3& getMaxVelocity(std::uint32_t _index) const;
        const glm::vec3& getAcceleration(std::uint32_t _index) const;
        const glm::vec3& getDeceleration(std::uint32_t _index) const;
        const glm::vec3& getMaxAcceleration(std::uint32_t _index) const;
        const glm::vec3& getMaxDeceleration(std::uint32_t _index) const;

        // linear property setters
        void setDirection(std::uint32_t _index, glm::vec3 _direction);
        void setPosition(std::uint32_t _index, glm::vec3 _position);
        void setVelocity(std::uint32_t _index, glm::vec3 _velocity);
        void setMaxVelocity(std::uint32_t _index, glm::vec3 _maxVelocity);
        void setAcceleration(std::uint32_t _index, glm::vec3 _acceleration);
        void setDeceleration(std::uint32_t _index, glm::vec3 _deceleration);
        void setMaxAcceleration(std::uint32_t _index, glm::vec3 _maxAcceleration);
        void setMaxDeceleration(std::uint32_t _index, glm::vec3 _maxDeceleration);

        // angular property getters
        float getAngle(std::uint32_t _index) const;
        float getAngularVelocity(std::uint32_t _index) const;

        // angular property setters
        void setAngle(std::uint32_t _index, float _angle);
        void setAngularVelocity(std::uint32_t _index, float _angularVelocity);

        // physics properties getters
        eBodyType  getBodyType(std::uint32_t _index) const;
        eShapeType getShapeType(std::uint32_t _index) const;
        float getRadius(std::uint32_t _index) const;
        float getInvMass(std::uint32_t _index) const;
        float getInvInertia(std::uint32_t _index) const;
        float getRestitution(std::uint32_t _index) const;
        float getFriction(std::uint32_t _index) const;

        // physics properties setters
        void setBodyType(std::uint32_t _index, eBodyType _type);
        void setShapeType(std::uint32_t _index, eShapeType _type);
        void setRadius(std::uint32_t _index, float _radius);
        void setMass(std::uint32_t _index, float _mass);          // sets invMass and invInertia automatically
        void setRestitution(std::uint32_t _index, float _restitution);
        void setFriction(std::uint32_t _index, float _friction);

        // direct access to component (for advanced use)
        sComponentPhysics* getComponent(std::uint32_t _index);

        // event interface
        sPhysicsEvent* getEvent(void) { return m_event.pop(); }

    private:
        // event
        tcQueue<sPhysicsEvent>         m_event = {};

        // components
        std::vector<sComponentPhysics> m_components;
        std::vector<std::uint32_t>     m_freeList;

        // external systems
        cEntitySystem*   m_entitySystem   = nullptr;
        cGraphicsSystem* m_graphicsSystem = nullptr;
};

#endif // PHYSICS_SYSTEM_HPP
