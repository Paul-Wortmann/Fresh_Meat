#include <cmath>
#include <algorithm>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/norm.hpp>

#include "physics_system.hpp"
#include "../entity_system/entity_system.hpp"
#include "../graphics_system/graphics_system.hpp"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ----------------------------------------------------------------------------
// Helper functions for collision detection
// ----------------------------------------------------------------------------

static inline glm::vec2 getHalfExtents(float radius)
{
    float half = radius * 0.5f;
    return glm::vec2(half, half);
}

static bool collideCircles(const glm::vec2& posA, float radA,
                           const glm::vec2& posB, float radB,
                           glm::vec2& normal, float& penetration)
{
    glm::vec2 delta = posB - posA;
    float dist2 = glm::dot(delta, delta);
    float radSum = radA + radB;
    if (dist2 >= radSum * radSum) return false;
    float dist = std::sqrt(dist2);
    if (dist < 1e-6f) normal = glm::vec2(1.0f, 0.0f);
    else normal = delta / dist;
    penetration = radSum - dist;
    return true;
}

static bool collideAABBs(const glm::vec2& minA, const glm::vec2& maxA,
                         const glm::vec2& minB, const glm::vec2& maxB,
                         glm::vec2& normal, float& penetration)
{
    glm::vec2 delta = (minB + maxB) * 0.5f - (minA + maxA) * 0.5f;
    glm::vec2 overlap = (maxA - minA) * 0.5f + (maxB - minB) * 0.5f - glm::abs(delta);
    if (overlap.x <= 0.0f || overlap.y <= 0.0f) return false;
    if (overlap.x < overlap.y)
    {
        normal = glm::vec2(glm::sign(delta.x), 0.0f);
        penetration = overlap.x;
    }
    else
    {
        normal = glm::vec2(0.0f, glm::sign(delta.y));
        penetration = overlap.y;
    }
    return true;
}

static bool collideCircleAABB(const glm::vec2& center, float radius,
                              const glm::vec2& min, const glm::vec2& max,
                              glm::vec2& normal, float& penetration)
{
    glm::vec2 closest = glm::clamp(center, min, max);
    glm::vec2 delta = center - closest;
    float dist2 = glm::dot(delta, delta);
    if (dist2 >= radius * radius) return false;
    float dist = std::sqrt(dist2);
    if (dist < 1e-6f)
    {
        glm::vec2 centerToMin = center - min;
        glm::vec2 centerToMax = max - center;
        float dx = centerToMin.x < centerToMax.x ? -centerToMin.x : centerToMax.x;
        float dy = centerToMin.y < centerToMax.y ? -centerToMin.y : centerToMax.y;
        if (std::abs(dx) < std::abs(dy))
            normal = glm::vec2(glm::sign(dx), 0.0f);
        else
            normal = glm::vec2(0.0f, glm::sign(dy));
        penetration = std::min(std::abs(dx), std::abs(dy));
    }
    else
    {
        normal = delta / dist;
        penetration = radius - dist;
    }
    return true;
}

// ----------------------------------------------------------------------------
// Continuous collision detection (swept circle vs circle)
// ----------------------------------------------------------------------------

static bool sweepCircles(const glm::vec2& posA, float radA, const glm::vec2& velA,
                         const glm::vec2& posB, float radB, const glm::vec2& velB,
                         float delta, float& t, glm::vec2& normal)
{
    glm::vec2 relPos = posB - posA;
    glm::vec2 relVel = velB - velA;
    float radSum = radA + radB;
    float a = glm::dot(relVel, relVel);
    float b = 2.0f * glm::dot(relPos, relVel);
    float c = glm::dot(relPos, relPos) - radSum * radSum;
    if (a == 0.0f) return false; // no relative movement
    float disc = b * b - 4.0f * a * c;
    if (disc < 0.0f) return false;
    float sqrtDisc = std::sqrt(disc);
    float t1 = (-b - sqrtDisc) / (2.0f * a);
    float t2 = (-b + sqrtDisc) / (2.0f * a);
    if (t1 > t2) std::swap(t1, t2);
    if (t1 < 0.0f && t2 < 0.0f) return false;
    t = (t1 >= 0.0f) ? t1 : t2;
    if (t > delta) return false;
    glm::vec2 hitPosA = posA + velA * t;
    glm::vec2 hitPosB = posB + velB * t;
    normal = glm::normalize(hitPosB - hitPosA);
    return true;
}

// ----------------------------------------------------------------------------
// Collision resolution (improved)
// ----------------------------------------------------------------------------

static void resolveCollision(sComponentPhysics& a, sComponentPhysics& b,
                             const glm::vec2& normal, float penetration,
                             const glm::vec2& contact)
{
    if (a.bodyType == eBodyType::staticObject && b.bodyType == eBodyType::staticObject)
        return;

    glm::vec2 posA(a.position.x, a.position.z);
    glm::vec2 posB(b.position.x, b.position.z);
    glm::vec2 rA = contact - posA;
    glm::vec2 rB = contact - posB;
    float rALen = glm::length(rA);
    float rBLen = glm::length(rB);
    glm::vec2 tA(-rA.y, rA.x);
    glm::vec2 tB(-rB.y, rB.x);

    glm::vec3 velA(a.velocity.x, 0.0f, a.velocity.z);
    glm::vec3 velB(b.velocity.x, 0.0f, b.velocity.z);
    float angA = a.angularVelocity;
    float angB = b.angularVelocity;

    glm::vec3 angVelA = angA * glm::vec3(-rA.y, 0.0f, rA.x);
    glm::vec3 angVelB = angB * glm::vec3(-rB.y, 0.0f, rB.x);
    glm::vec3 relVel = (velB + angVelB) - (velA + angVelA);

    glm::vec3 normal3(normal.x, 0.0f, normal.y);
    float velAlongNormal = glm::dot(relVel, normal3);
    if (velAlongNormal > 0.0f) return;

    float invMassSum = a.invMass + b.invMass;
    float angInvA = rALen * rALen * a.invInertia;
    float angInvB = rBLen * rBLen * b.invInertia;
    float totalInvMass = invMassSum + angInvA + angInvB;
    if (totalInvMass == 0.0f) return;

    float e = std::min(a.restitution, b.restitution);
    float j = -(1.0f + e) * velAlongNormal / totalInvMass;
    glm::vec3 impulseNormal = j * normal3;

    a.velocity -= impulseNormal * a.invMass;
    b.velocity += impulseNormal * b.invMass;

    // Correct torque calculation using cross product
    glm::vec3 rA3(rA.x, 0.0f, rA.y);
    glm::vec3 rB3(rB.x, 0.0f, rB.y);
    float torqueA = glm::cross(rA3, impulseNormal).y;  // y component = scalar torque
    float torqueB = glm::cross(rB3, impulseNormal).y;
    a.angularVelocity -= torqueA * a.invInertia;
    b.angularVelocity += torqueB * b.invInertia;

    // Friction (using updated angular velocities)
    glm::vec3 velANew(a.velocity.x, 0.0f, a.velocity.z);
    glm::vec3 velBNew(b.velocity.x, 0.0f, b.velocity.z);
    glm::vec3 angVelA_new = a.angularVelocity * glm::vec3(-rA.y, 0.0f, rA.x);
    glm::vec3 angVelB_new = b.angularVelocity * glm::vec3(-rB.y, 0.0f, rB.x);
    glm::vec3 relVelNew = (velBNew + angVelB_new) - (velANew + angVelA_new);
    glm::vec3 tangentVec = relVelNew - glm::dot(relVelNew, normal3) * normal3;
    float tangentSpeed = glm::length(tangentVec);
    if (tangentSpeed > 1e-6f)
    {
        glm::vec3 tangentDir = tangentVec / tangentSpeed;
        float frictionCoeff = std::sqrt(a.friction * b.friction);
        float jtMax = frictionCoeff * j;
        float jt = -tangentSpeed / totalInvMass;
        if (std::abs(jt) > jtMax) jt = (jt > 0.0f ? jtMax : -jtMax);
        glm::vec3 impulseTangent = jt * tangentDir;

        a.velocity -= impulseTangent * a.invMass;
        b.velocity += impulseTangent * b.invMass;
        glm::vec2 ft2D(impulseTangent.x, impulseTangent.z);
        float torqueA_f = glm::dot(ft2D, tA);
        float torqueB_f = glm::dot(ft2D, tB);
        a.angularVelocity -= torqueA_f * a.invInertia;
        b.angularVelocity += torqueB_f * b.invInertia;
    }

    // Positional correction (fixed typo: penatration -> penetration)
    const float percent = 0.2f;
    const float slop = 0.01f;
    float correctionMagnitude = std::max(penetration - slop, 0.0f) / (invMassSum + 1e-6f) * percent;
    glm::vec3 correctionVec = correctionMagnitude * normal3;
    if (a.invMass > 0.0f) a.position -= correctionVec * a.invMass;
    if (b.invMass > 0.0f) b.position += correctionVec * b.invMass;
}

// ----------------------------------------------------------------------------
// cPhysicsSystem implementation
// ----------------------------------------------------------------------------

bool cPhysicsSystem::initialize(void)
{
    m_components.clear();
    m_freeList.clear();
    return true;
}

void cPhysicsSystem::terminate(void)
{
    for (sPhysicsEvent* tEvent = getEvent(); tEvent != nullptr; tEvent = getEvent())
        delete tEvent;
    m_components.clear();
    m_freeList.clear();
}

void cPhysicsSystem::process(float _delta)
{
    // --------------------------------------------------------------
    // 1. Continuous collision detection (CCD) for circle vs circle
    // --------------------------------------------------------------
    float remainingDelta = _delta;
    const int ccdIterations = 4;
    for (int ccd = 0; ccd < ccdIterations && remainingDelta > 1e-6f; ++ccd)
    {
        float earliestT = remainingDelta;
        int idxA = -1, idxB = -1;
        glm::vec2 earliestNormal(0.0f);

        // Find earliest collision among all dynamic circles
        for (std::uint32_t i = 0; i < m_components.size(); ++i)
        {
            if (!m_components[i].enabled) continue;
            if (m_components[i].bodyType != eBodyType::dynamicObject) continue;
            if (m_components[i].shapeType != eShapeType::circle) continue;

            for (std::uint32_t j = i + 1; j < m_components.size(); ++j)
            {
                if (!m_components[j].enabled) continue;
                if (m_components[j].bodyType != eBodyType::dynamicObject) continue;
                if (m_components[j].shapeType != eShapeType::circle) continue;

                float t;
                glm::vec2 normal;
                glm::vec2 posA(m_components[i].position.x, m_components[i].position.z);
                glm::vec2 posB(m_components[j].position.x, m_components[j].position.z);
                glm::vec2 velA(m_components[i].velocity.x, m_components[i].velocity.z);
                glm::vec2 velB(m_components[j].velocity.x, m_components[j].velocity.z);

                if (sweepCircles(posA, m_components[i].radius, velA,
                                 posB, m_components[j].radius, velB,
                                 remainingDelta, t, normal))
                {
                    if (t < earliestT)
                    {
                        earliestT = t;
                        idxA = i;
                        idxB = j;
                        earliestNormal = normal;
                    }
                }
            }
        }

        if (idxA == -1) break; // no more collisions in this time step

        // Move all dynamic objects forward to the time of collision
        for (auto& comp : m_components)
        {
            if (!comp.enabled) continue;
            if (comp.bodyType == eBodyType::dynamicObject)
            {
                comp.position += comp.velocity * earliestT;
                comp.angle += comp.angularVelocity * earliestT;
                // keep angle in range
                comp.angle = fmod(comp.angle, 2.0f * static_cast<float>(M_PI));
            }
        }
        remainingDelta -= earliestT;

        // Resolve this collision
        sComponentPhysics& a = m_components[idxA];
        sComponentPhysics& b = m_components[idxB];
        glm::vec2 posA(a.position.x, a.position.z);
        glm::vec2 posB(b.position.x, b.position.z);
        float dist = glm::distance(posA, posB);
        float penetration = (a.radius + b.radius) - dist;
        if (penetration > 0.0f)
        {
            glm::vec2 contact = posA + earliestNormal * (a.radius - penetration * 0.5f);
            resolveCollision(a, b, earliestNormal, penetration, contact);
        }

        // Push collision event
        sPhysicsEvent* evt = new sPhysicsEvent;
        evt->type = ePhysicsEventType::collision;
        evt->bodyA.componentID = idxA;
        evt->bodyA.bodyType    = a.bodyType;
        evt->bodyB.componentID = idxB;
        evt->bodyB.bodyType    = b.bodyType;
        m_event.push(evt);
    }

    // --------------------------------------------------------------
    // 2. Integrate remaining motion (no more CCD collisions)
    // --------------------------------------------------------------
    for (auto& comp : m_components)
    {
        if (!comp.enabled) continue;

        // Update velocity from acceleration
        comp.velocity += comp.acceleration * remainingDelta;

        // Apply deceleration if no acceleration is active
        if (glm::length2(comp.acceleration) < 1e-12f && glm::length2(comp.velocity) > 1e-12f)
        {
            glm::vec3 velDir = glm::normalize(comp.velocity);
            glm::vec3 decel = comp.deceleration * remainingDelta;

            // Ensure decel does not exceed max deceleration per axis
            if (comp.maxDeceleration.x > 0.0f)
                decel.x = glm::min(decel.x, comp.maxDeceleration.x * remainingDelta);
            if (comp.maxDeceleration.z > 0.0f)
                decel.z = glm::min(decel.z, comp.maxDeceleration.z * remainingDelta);

            // Subtract deceleration opposite to velocity direction
            glm::vec3 deltaV = decel * velDir;
            if (glm::length2(deltaV) > glm::length2(comp.velocity))
                comp.velocity = glm::vec3(0.0f);  // would overshoot zero
            else
                comp.velocity -= deltaV;
        }

        // Clamp velocity to maxVelocity
        if (comp.maxVelocity.x > 0.0f)
            comp.velocity.x = glm::clamp(comp.velocity.x, -comp.maxVelocity.x, comp.maxVelocity.x);
        if (comp.maxVelocity.z > 0.0f)
            comp.velocity.z = glm::clamp(comp.velocity.z, -comp.maxVelocity.z, comp.maxVelocity.z);

        // Integrate position
        comp.position += comp.velocity * remainingDelta;

        // Angular motion
        comp.angle += comp.angularVelocity * remainingDelta;
        comp.angle = fmod(comp.angle, 2.0f * static_cast<float>(M_PI));
    }

    // --------------------------------------------------------------
    // 3. Iterative overlap resolution (static/dynamic + AABB)
    // --------------------------------------------------------------
    const int iterations = 6;
    for (int iter = 0; iter < iterations; ++iter)
    {
        for (std::uint32_t i = 0; i < m_components.size(); ++i)
        {
            if (!m_components[i].enabled) continue;
            for (std::uint32_t j = i + 1; j < m_components.size(); ++j)
            {
                if (!m_components[j].enabled) continue;

                sComponentPhysics& a = m_components[i];
                sComponentPhysics& b = m_components[j];

                if (a.bodyType == eBodyType::staticObject && b.bodyType == eBodyType::staticObject)
                    continue;

                glm::vec2 normal(0.0f);
                float penetration = 0.0f;
                bool colliding = false;

                if (a.shapeType == eShapeType::circle && b.shapeType == eShapeType::circle)
                {
                    colliding = collideCircles(glm::vec2(a.position.x, a.position.z), a.radius,
                                               glm::vec2(b.position.x, b.position.z), b.radius,
                                               normal, penetration);
                }
                else if (a.shapeType == eShapeType::aabb && b.shapeType == eShapeType::aabb)
                {
                    glm::vec2 halfA = getHalfExtents(a.radius);
                    glm::vec2 halfB = getHalfExtents(b.radius);
                    glm::vec2 minA = glm::vec2(a.position.x - halfA.x, a.position.z - halfA.y);
                    glm::vec2 maxA = glm::vec2(a.position.x + halfA.x, a.position.z + halfA.y);
                    glm::vec2 minB = glm::vec2(b.position.x - halfB.x, b.position.z - halfB.y);
                    glm::vec2 maxB = glm::vec2(b.position.x + halfB.x, b.position.z + halfB.y);
                    colliding = collideAABBs(minA, maxA, minB, maxB, normal, penetration);
                }
                else
                {
                    sComponentPhysics& circle = (a.shapeType == eShapeType::circle) ? a : b;
                    sComponentPhysics& aabb   = (a.shapeType == eShapeType::circle) ? b : a;
                    glm::vec2 half = getHalfExtents(aabb.radius);
                    glm::vec2 min = glm::vec2(aabb.position.x - half.x, aabb.position.z - half.y);
                    glm::vec2 max = glm::vec2(aabb.position.x + half.x, aabb.position.z + half.y);
                    colliding = collideCircleAABB(glm::vec2(circle.position.x, circle.position.z),
                                                  circle.radius, min, max, normal, penetration);
                    if (&circle == &a)
                    {
                        // normal already points from circle to AABB
                    }
                    else
                    {
                        normal = -normal;
                    }
                }

                if (colliding)
                {
                    // Emit event if exactly one is static
                    if ((a.bodyType == eBodyType::dynamicObject && b.bodyType == eBodyType::staticObject) ||
                        (a.bodyType == eBodyType::staticObject && b.bodyType == eBodyType::dynamicObject))
                    {
                        sPhysicsEvent* evt = new sPhysicsEvent;
                        evt->type = ePhysicsEventType::collision;
                        evt->bodyA.componentID = i;
                        evt->bodyA.bodyType    = a.bodyType;
                        evt->bodyB.componentID = j;
                        evt->bodyB.bodyType    = b.bodyType;
                        m_event.push(evt);
                    }

                    glm::vec2 posA(a.position.x, a.position.z);
                    glm::vec2 posB(b.position.x, b.position.z);
                    glm::vec2 contact = posA + normal * (penetration * 0.5f);
                    resolveCollision(a, b, normal, penetration, contact);
                }
            }
        }
    }

    // --------------------------------------------------------------
    // 4. Update direction from velocity (for rendering)
    // --------------------------------------------------------------
    for (auto& comp : m_components)
    {
        if (!comp.enabled) continue;
        if (glm::length(comp.velocity) > 0.001f)
        {
            comp.direction = glm::normalize(comp.velocity);
        }
    }

    // --------------------------------------------------------------
    // 5. Update graphics transforms for all dynamic objects
    // --------------------------------------------------------------
    if (m_entitySystem && m_graphicsSystem)
    {
        for (std::uint32_t i = 0; i < m_components.size(); ++i)
        {
            const auto& comp = m_components[i];
            if (!comp.enabled) continue;
            if (comp.bodyType == eBodyType::dynamicObject)
            {
                int32_t entityID = m_entitySystem->getEntityIDFromPhysicsID(i);
                if (entityID != -1)
                {
                    m_graphicsSystem->updateComponentMatrix(entityID, comp.position, comp.direction);
                }
            }
        }
    }
}

// ----------------------------------------------------------------------------
// Component management
// ----------------------------------------------------------------------------

std::uint32_t cPhysicsSystem::getNewComponent(void)
{
    if (!m_freeList.empty())
    {
        std::uint32_t index = m_freeList.back();
        m_freeList.pop_back();
        m_components[index] = sComponentPhysics{};
        m_components[index].enabled = true;
        return index;
    }
    m_components.emplace_back();
    m_components.back().enabled = true;
    return static_cast<std::uint32_t>(m_components.size() - 1);
}

void cPhysicsSystem::destroyComponent(const std::uint32_t& _index)
{
    if (_index >= m_components.size()) return;
    if (!m_components[_index].enabled) return;
    m_components[_index].enabled = false;
    m_freeList.push_back(_index);
}

// ----------------------------------------------------------------------------
// Getters / Setters
// ----------------------------------------------------------------------------

const glm::vec3& cPhysicsSystem::getDirection(std::uint32_t _index) const
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    return m_components[_index].direction;
}
const glm::vec3& cPhysicsSystem::getPosition(std::uint32_t _index) const
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    return m_components[_index].position;
}
const glm::vec3& cPhysicsSystem::getVelocity(std::uint32_t _index) const
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    return m_components[_index].velocity;
}
const glm::vec3& cPhysicsSystem::getMaxVelocity(std::uint32_t _index) const
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    return m_components[_index].maxVelocity;
}
const glm::vec3& cPhysicsSystem::getDeceleration(std::uint32_t _index) const
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    return m_components[_index].deceleration;
}
const glm::vec3& cPhysicsSystem::getAcceleration(std::uint32_t _index) const
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    return m_components[_index].acceleration;
}
const glm::vec3& cPhysicsSystem::getMaxAcceleration(std::uint32_t _index) const
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    return m_components[_index].maxAcceleration;
}
const glm::vec3& cPhysicsSystem::getMaxDeceleration(std::uint32_t _index) const
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    return m_components[_index].maxDeceleration;
}

void cPhysicsSystem::setDirection(std::uint32_t _index, glm::vec3 _direction)
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    m_components[_index].direction = _direction;
}
void cPhysicsSystem::setPosition(std::uint32_t _index, glm::vec3 _position)
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    m_components[_index].position = _position;
}
void cPhysicsSystem::setVelocity(std::uint32_t _index, glm::vec3 _velocity)
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    m_components[_index].velocity = _velocity;
}
void cPhysicsSystem::setMaxVelocity(std::uint32_t _index, glm::vec3 _maxVelocity)
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    m_components[_index].maxVelocity = _maxVelocity;
}
void cPhysicsSystem::setAcceleration(std::uint32_t _index, glm::vec3 _acceleration)
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    m_components[_index].acceleration = _acceleration;
}
void cPhysicsSystem::setDeceleration(std::uint32_t _index, glm::vec3 _deceleration)
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    m_components[_index].deceleration = _deceleration;
}
void cPhysicsSystem::setMaxAcceleration(std::uint32_t _index, glm::vec3 _maxAcceleration)
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    m_components[_index].maxAcceleration = _maxAcceleration;
}
void cPhysicsSystem::setMaxDeceleration(std::uint32_t _index, glm::vec3 _maxDeceleration)
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    m_components[_index].maxDeceleration = _maxDeceleration;
}

float cPhysicsSystem::getAngle(std::uint32_t _index) const
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    return m_components[_index].angle;
}
float cPhysicsSystem::getAngularVelocity(std::uint32_t _index) const
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    return m_components[_index].angularVelocity;
}
void cPhysicsSystem::setAngle(std::uint32_t _index, float _angle)
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    m_components[_index].angle = _angle;
}
void cPhysicsSystem::setAngularVelocity(std::uint32_t _index, float _angularVelocity)
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    m_components[_index].angularVelocity = _angularVelocity;
}

eBodyType cPhysicsSystem::getBodyType(std::uint32_t _index) const
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    return m_components[_index].bodyType;
}
eShapeType cPhysicsSystem::getShapeType(std::uint32_t _index) const
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    return m_components[_index].shapeType;
}
float cPhysicsSystem::getRadius(std::uint32_t _index) const
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    return m_components[_index].radius;
}
float cPhysicsSystem::getInvMass(std::uint32_t _index) const
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    return m_components[_index].invMass;
}
float cPhysicsSystem::getInvInertia(std::uint32_t _index) const
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    return m_components[_index].invInertia;
}
float cPhysicsSystem::getRestitution(std::uint32_t _index) const
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    return m_components[_index].restitution;
}
float cPhysicsSystem::getFriction(std::uint32_t _index) const
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    return m_components[_index].friction;
}

void cPhysicsSystem::setBodyType(std::uint32_t _index, eBodyType _type)
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    m_components[_index].bodyType = _type;
    if (_type == eBodyType::staticObject)
    {
        m_components[_index].invMass = 0.0f;
        m_components[_index].invInertia = 0.0f;
    }
}
void cPhysicsSystem::setShapeType(std::uint32_t _index, eShapeType _type)
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    m_components[_index].shapeType = _type;
}
void cPhysicsSystem::setRadius(std::uint32_t _index, float _radius)
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    m_components[_index].radius = _radius;
}
void cPhysicsSystem::setMass(std::uint32_t _index, float _mass)
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    if (m_components[_index].bodyType == eBodyType::staticObject)
    {
        m_components[_index].invMass = 0.0f;
        m_components[_index].invInertia = 0.0f;
        return;
    }
    if (_mass <= 0.0f) _mass = 0.0001f;
    m_components[_index].invMass = 1.0f / _mass;
    float r = m_components[_index].radius;
    if (m_components[_index].shapeType == eShapeType::circle)
    {
        float inertia = 0.5f * _mass * r * r;
        m_components[_index].invInertia = (inertia > 0.0f) ? 1.0f / inertia : 0.0f;
    }
    else
    {
        float inertia = (1.0f / 6.0f) * _mass * r * r;
        m_components[_index].invInertia = (inertia > 0.0f) ? 1.0f / inertia : 0.0f;
    }
}
void cPhysicsSystem::setRestitution(std::uint32_t _index, float _restitution)
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    m_components[_index].restitution = glm::clamp(_restitution, 0.0f, 1.0f);
}
void cPhysicsSystem::setFriction(std::uint32_t _index, float _friction)
{
    assert(_index < m_components.size() && m_components[_index].enabled);
    m_components[_index].friction = glm::clamp(_friction, 0.0f, 1.0f);
}

sComponentPhysics* cPhysicsSystem::getComponent(std::uint32_t _index)
{
    if (_index >= m_components.size()) return nullptr;
    if (!m_components[_index].enabled) return nullptr;
    return &m_components[_index];
}
