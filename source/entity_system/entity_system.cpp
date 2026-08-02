#include "entity_system.hpp"
#include "../physics_system/physics_component_define.hpp" // for eShapeType, eBodyType
#include "../audio_system/audio_system.hpp"
#include "../graphics_system/graphics_system.hpp"
#include "../physics_system/physics_system.hpp"

// ----------------------------------------------------------------------------
// Helper lambdas for XML parsing (same as in original)
// ----------------------------------------------------------------------------

static auto extractString = [](const std::string &line) -> std::string
{
    size_t start = line.find('>');
    size_t end   = line.rfind('<');
    if (start == std::string::npos || end == std::string::npos || start + 1 >= end)
        return {};
    return line.substr(start + 1, end - start - 1);
};

static auto extractVec3 = [&extractString](const std::string &line) -> glm::vec3
{
    std::string value = extractString(line);
    if (value.empty())
        return glm::vec3(0.0f);
    std::stringstream ss(value);
    float x, y, z;
    char comma;
    if (!(ss >> x >> comma >> y >> comma >> z) || comma != ',')
        return glm::vec3(0.0f);
    return glm::vec3(x, y, z);
};

static auto extractFloat = [&extractString](const std::string &line) -> float
{
    std::string value = extractString(line);
    if (value.empty())
        return 0.0f;
    std::stringstream ss(value);
    float f;
    ss >> f;
    return f;
};

static auto extractInt32 = [&extractString](const std::string &line) -> std::int32_t
{
    std::string value = extractString(line);
    if (value.empty())
        return 0;
    std::stringstream ss(value);
    std::int32_t i;
    ss >> i;
    return i;
};

// ----------------------------------------------------------------------------
// cEntitySystem implementation
// ----------------------------------------------------------------------------

bool cEntitySystem::initialize(void)
{
    m_entity.clear();
    m_freeList.clear();
    m_physicsToEntity.clear();
    m_playerIndex = -1;
    return true;
}

void cEntitySystem::terminate(void)
{
    // free all events
    for (sEntityEvent* tEvent = getEvent(); tEvent != nullptr; tEvent = getEvent())
        delete tEvent;

    // Destroy all entities
    for (std::size_t i = 0; i < m_entity.size(); ++i)
    {
        if (m_entity[i].enabled)
            destroyEntity(static_cast<std::int32_t>(i));
    }
    m_entity.clear();
    m_freeList.clear();
    m_physicsToEntity.clear();
}

void cEntitySystem::process(float _delta)
{
    // Nothing to do here by default – physics and graphics are updated in their own systems
    (void)_delta;
}

std::uint32_t cEntitySystem::getNewEntity(void)
{
    if (!m_freeList.empty())
    {
        std::uint32_t index = m_freeList.back();
        m_freeList.pop_back();
        m_entity[index] = sEntity{};
        m_entity[index].enabled = true;
        return index;
    }
    m_entity.emplace_back();
    m_entity.back().enabled = true;
    return static_cast<std::uint32_t>(m_entity.size() - 1);
}

// ----------------------------------------------------------------------------
// Destroy entity:
// ----------------------------------------------------------------------------
void cEntitySystem::destroyEntity(const std::int32_t &_index)
{
    if (_index < 0 || _index >= static_cast<std::int32_t>(m_entity.size()))
        return;
    if (!m_entity[_index].enabled)
        return;

    // Remove mapping before destroying the physics component
    if (m_entity[_index].physicsComponent != -1)
    {
        m_physicsToEntity.erase(m_entity[_index].physicsComponent);
    }

    if (m_audioSystem && m_entity[_index].audioComponent != -1)
    {
        m_audioSystem->freeComponent(m_entity[_index].audioComponent);
        m_entity[_index].audioComponent = -1;
    }
    if (m_graphicsSystem && m_entity[_index].graphicsComponent != -1)
    {
        m_graphicsSystem->releaseComponent(m_entity[_index].graphicsComponent);
        m_entity[_index].graphicsComponent = -1;
    }
    if (m_physicsSystem && m_entity[_index].physicsComponent != -1)
    {
        m_physicsSystem->destroyComponent(m_entity[_index].physicsComponent);
        m_entity[_index].physicsComponent = -1;
    }

    m_entity[_index].enabled = false;
    m_freeList.push_back(_index);

    if (m_playerIndex == _index)
        m_playerIndex = -1;
}

// ----------------------------------------------------------------------------
// getEntityIDFromPhysicsID: get entity index from physics component index
// ----------------------------------------------------------------------------
std::int32_t cEntitySystem::getEntityIDFromPhysicsID(const std::int32_t &_physicsIndex) const
{
    if (_physicsIndex == -1)
        return -1;

    auto it = m_physicsToEntity.find(_physicsIndex);
    if (it == m_physicsToEntity.end())
        return -1;

    // Optional: validate that the entity still exists and still references the same physics component
    std::int32_t entityIdx = it->second;
    if (entityIdx < 0 || entityIdx >= static_cast<std::int32_t>(m_entity.size()) ||
        !m_entity[entityIdx].enabled ||
        m_entity[entityIdx].physicsComponent != _physicsIndex)
    {
        // Inconsistent state – should not happen, but we return -1 and could log a warning
        return -1;
    }

    return entityIdx;
}

bool cEntitySystem::hasAudio(std::uint32_t _entity) const
{
    if (_entity >= m_entity.size()) return false;
    if (!m_entity[_entity].enabled) return false;
    return m_entity[_entity].audioComponent != -1;
}

bool cEntitySystem::hasGraphics(std::uint32_t _entity) const
{
    if (_entity >= m_entity.size()) return false;
    if (!m_entity[_entity].enabled) return false;
    return m_entity[_entity].graphicsComponent != -1;
}

bool cEntitySystem::hasPhysics(std::uint32_t _entity) const
{
    if (_entity >= m_entity.size()) return false;
    if (!m_entity[_entity].enabled) return false;
    return m_entity[_entity].physicsComponent != -1;
}

// ----------------------------------------------------------------------------
// Load entity from XML file
// ----------------------------------------------------------------------------

std::int32_t cEntitySystem::loadEntity(const std::string &_fileName, const glm::vec3 &_position)
{
    std::ifstream file(_fileName);
    if (!file.is_open())
    {
        std::cerr << "[EntitySystem] Failed to open file: " << _fileName << std::endl;
        return -1;
    }

    // Temporary storage for parsed data
    std::string   entity_name;
    std::uint32_t entity_type = 0;

    std::int32_t  animation_idle = -1;
    std::int32_t  animation_walk = -1;

    std::string   audio_spawn;

    std::string   graphics_model;
    glm::vec3     graphics_scale(1.0f);

    glm::vec3     physics_position(0.0f);
    glm::vec3     physics_direction(0.0f);
    glm::vec3     physics_velocity(0.0f);
    glm::vec3     physics_max_velocity(0.0f);
    glm::vec3     physics_acceleration(0.0f);
    glm::vec3     physics_deceleration(0.0f);
    glm::vec3     physics_max_acceleration(0.0f);
    glm::vec3     physics_max_deceleration(0.0f);
    std::string   physics_shape = "circle";
    float         physics_radius = 1.0f;
    std::string   physics_body_type = "none";
    float         physics_mass = 1.0f;
    float         physics_restitution = 0.5f;
    float         physics_friction = 0.5f;
    float         physics_angle = 0.0f;
    float         physics_angular_velocity = 0.0f;

    // Material data
    struct MaterialData
    {
        std::string diffuse;
        std::string normal;
        std::string specular;
    };
    std::vector<MaterialData> materials;
    bool inMaterial = false;
    MaterialData currentMaterial;

    // Mesh data
    struct MeshData
    {
        std::string name = {};
        bool enabled = true;
    };
    std::vector<MeshData> meshes;
    bool inMesh = false;
    MeshData currentMesh;

    std::string line;
    while (std::getline(file, line))
    {
        // Trim whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        size_t end = line.find_last_not_of(" \t\r\n");
        line = line.substr(start, end - start + 1);

        // Material block handling
        if (line == "<material>")
        {
            inMaterial = true;
            currentMaterial = MaterialData{};
            continue;
        }
        if (line == "</material>")
        {
            inMaterial = false;
            materials.push_back(currentMaterial);
            continue;
        }

        // Mesh block handling
        if (line == "<mesh>")
        {
            inMesh = true;
            currentMesh = MeshData{};
            continue;
        }
        if (line == "</mesh>")
        {
            inMesh = false;
            meshes.push_back(currentMesh);
            continue;
        }

        // Extract tag name
        size_t tag_start = line.find('<');
        size_t tag_end   = line.find('>');
        if (tag_start == std::string::npos || tag_end == std::string::npos)
            continue;
        std::string tag = line.substr(tag_start + 1, tag_end - tag_start - 1);

        if (inMaterial)
        {
            if (tag == "texture_diffuse")
                currentMaterial.diffuse = extractString(line);
            else if (tag == "texture_normal")
                currentMaterial.normal = extractString(line);
            else if (tag == "texture_specular")
                currentMaterial.specular = extractString(line);
        }
        else if (inMesh)
        {
            if (tag == "mesh_name")
                currentMesh.name = extractString(line);
            else if (tag == "mesh_enabled")
                currentMesh.enabled = (extractInt32(line) > 0) ? true : false;
        }
        else
        {
            // Base / graphics / physics tags
            if (tag == "entity_name")
                entity_name = extractString(line);
            else if (tag == "entity_type")
                entity_type = extractInt32(line);
            else if (tag == "animation_idle")
                animation_idle = extractInt32(line);
            else if (tag == "animation_walk")
                animation_walk = extractInt32(line);
            else if (tag == "audio_spawn")
                audio_spawn = extractString(line);
            else if (tag == "graphics_model")
                graphics_model = extractString(line);
            else if (tag == "graphics_scale")
                graphics_scale = extractVec3(line);
            else if (tag == "physics_position")
                physics_position = extractVec3(line);
            else if (tag == "physics_direction")
                physics_direction = extractVec3(line);
            else if (tag == "physics_velocity")
                physics_velocity = extractVec3(line);
            else if (tag == "physics_max_velocity")
                physics_max_velocity = extractVec3(line);
            else if (tag == "physics_acceleration")
                physics_acceleration = extractVec3(line);
            else if (tag == "physics_deceleration")
                physics_deceleration = extractVec3(line);
            else if (tag == "physics_max_acceleration")
                physics_max_acceleration = extractVec3(line);
            else if (tag == "physics_max_deceleration")
                physics_max_deceleration = extractVec3(line);
            else if (tag == "physics_shape")
                physics_shape = extractString(line);
            else if (tag == "physics_radius")
                physics_radius = extractFloat(line);
            else if (tag == "physics_body_type")
                physics_body_type = extractString(line);
            else if (tag == "physics_mass")
                physics_mass = extractFloat(line);
            else if (tag == "physics_restitution")
                physics_restitution = extractFloat(line);
            else if (tag == "physics_friction")
                physics_friction = extractFloat(line);
            else if (tag == "physics_angle")
                physics_angle = extractFloat(line);
            else if (tag == "physics_angular_velocity")
                physics_angular_velocity = extractFloat(line);
        }
    }
    file.close();

    // Create entity
    std::uint32_t entIdx = getNewEntity();
    m_entity[entIdx].type = static_cast<eEntityType>(entity_type);
    if (m_entity[entIdx].type == eEntityType::player)
    {
        m_playerIndex = static_cast<std::int32_t>(entIdx);

        // push player found event
        sEntityEvent* tEvent = new sEntityEvent;
        tEvent->type = eEntityEventType::playerLoaded;
        tEvent->data = entIdx;
        m_event.push(tEvent);
    }
    else if (m_entity[entIdx].type == eEntityType::npc)
    {
        // push npc found event
        sEntityEvent* tEvent = new sEntityEvent;
        tEvent->type = eEntityEventType::npcLoaded;
        tEvent->data = entIdx;
        m_event.push(tEvent);
    }

    // --- Physics component ---
    if (m_physicsSystem)
    {
        m_entity[entIdx].physicsComponent = m_physicsSystem->getNewComponent();

        // Shape
        if (physics_shape == "aabb")
            m_physicsSystem->setShapeType(m_entity[entIdx].physicsComponent, eShapeType::aabb);
        else
            m_physicsSystem->setShapeType(m_entity[entIdx].physicsComponent, eShapeType::circle);

        m_physicsSystem->setRadius(m_entity[entIdx].physicsComponent, physics_radius);

        // Body type & mass
        eBodyType bodyType = eBodyType::noneObject;
        if (physics_body_type == "static")
            bodyType = eBodyType::staticObject;
        else if (physics_body_type == "dynamic")
            bodyType = eBodyType::dynamicObject;
        m_physicsSystem->setBodyType(m_entity[entIdx].physicsComponent, bodyType);

        if (bodyType == eBodyType::dynamicObject && physics_mass > 0.0f)
            m_physicsSystem->setMass(m_entity[entIdx].physicsComponent, physics_mass);
        // For static, invMass/invInertia remain 0 (set by setBodyType)

        m_physicsSystem->setRestitution(m_entity[entIdx].physicsComponent, physics_restitution);
        m_physicsSystem->setFriction(m_entity[entIdx].physicsComponent, physics_friction);
        m_physicsSystem->setAngle(m_entity[entIdx].physicsComponent, physics_angle);
        m_physicsSystem->setAngularVelocity(m_entity[entIdx].physicsComponent, physics_angular_velocity);

        // Linear motion
        m_physicsSystem->setPosition(m_entity[entIdx].physicsComponent, physics_position + _position);
        m_physicsSystem->setVelocity(m_entity[entIdx].physicsComponent, physics_velocity);
        m_physicsSystem->setAcceleration(m_entity[entIdx].physicsComponent, physics_acceleration);
        m_physicsSystem->setDeceleration(m_entity[entIdx].physicsComponent, physics_deceleration);
        m_physicsSystem->setMaxVelocity(m_entity[entIdx].physicsComponent, physics_max_velocity);
        m_physicsSystem->setMaxAcceleration(m_entity[entIdx].physicsComponent, physics_max_acceleration);
        m_physicsSystem->setMaxDeceleration(m_entity[entIdx].physicsComponent, physics_max_deceleration);
        m_physicsSystem->setDirection(m_entity[entIdx].physicsComponent, physics_direction);

        //std::cout << "max Velocity: " << physics_max_velocity.x << " " << physics_max_velocity.y << " " << physics_max_velocity.z << std::endl;
        //std::cout << "max Acceleration: " << physics_max_acceleration.x << " " << physics_max_acceleration.y << " " << physics_max_acceleration.z << std::endl;
        //std::cout << "max Deceleration: " << physics_max_deceleration.x << " " << physics_max_deceleration.y << " " << physics_max_deceleration.z << std::endl;

        m_physicsToEntity[m_entity[entIdx].physicsComponent] = static_cast<std::int32_t>(entIdx);
    }

    // --- Graphics component ---
    if (m_graphicsSystem && !graphics_model.empty())
    {
        m_entity[entIdx].graphicsComponent = m_graphicsSystem->getNewComponent();
        m_graphicsSystem->setScale(m_entity[entIdx].graphicsComponent, graphics_scale);
        m_graphicsSystem->loadModel(m_entity[entIdx].graphicsComponent, FILE_PATH_MODEL + graphics_model);

        // Animation
        m_graphicsSystem->setAnimationIndex(m_entity[entIdx].graphicsComponent, eAnimationType::idle, animation_idle);
        m_graphicsSystem->setAnimationIndex(m_entity[entIdx].graphicsComponent, eAnimationType::walk, animation_walk);

        // Load materials (textures)
        for (std::size_t matIdx = 0; matIdx < materials.size(); ++matIdx)
        {
            const auto& mat = materials[matIdx];
            if (!mat.diffuse.empty())
                m_graphicsSystem->loadTextureDiffuse(m_entity[entIdx].graphicsComponent, FILE_PATH_TEXTURE + mat.diffuse, GL_CLAMP_TO_EDGE, static_cast<std::uint32_t>(matIdx));
            if (!mat.normal.empty())
                m_graphicsSystem->loadTextureNormal(m_entity[entIdx].graphicsComponent, FILE_PATH_TEXTURE + mat.normal, GL_CLAMP_TO_EDGE, static_cast<std::uint32_t>(matIdx));
            if (!mat.specular.empty())
                m_graphicsSystem->loadTextureSpecular(m_entity[entIdx].graphicsComponent, FILE_PATH_TEXTURE + mat.specular, GL_CLAMP_TO_EDGE, static_cast<std::uint32_t>(matIdx));
        }

        // process mesh data
        for (std::size_t meshIdx = 0; meshIdx < meshes.size(); ++meshIdx)
        {
            m_graphicsSystem->setMeshName(m_entity[entIdx].graphicsComponent, meshIdx, meshes[meshIdx].name);
            m_graphicsSystem->setMeshEnabled(m_entity[entIdx].graphicsComponent, meshIdx, meshes[meshIdx].enabled);
        }

        // Update graphics transform (position + direction)
        m_graphicsSystem->updateComponentMatrix(m_entity[entIdx].graphicsComponent, physics_position + _position, physics_direction);
    }

    // --- Audio component (optional) ---
    if (m_audioSystem && !audio_spawn.empty())
    {
        // we shouldn't store the ID in here directly, will change this later
        m_entity[entIdx].audioComponent = m_audioSystem->loadSound(audio_spawn);
    }

    return static_cast<std::int32_t>(entIdx);
}
