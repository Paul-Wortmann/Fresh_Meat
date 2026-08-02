#include "map_system.hpp"

bool cMapSystem::initialize(void)
{
    return true;
}

void cMapSystem::terminate(void)
{
    for (sMapEvent* tEvent = getEvent(); tEvent != nullptr; tEvent = getEvent())
        delete tEvent;
    freeMap();
}

void cMapSystem::setProcessState(const eMapState &_state)
{
    // Clear all tile highlights (x) and selections (y)
    for (uint32_t i = 0; i < m_map.numTiles; ++i)
    {
        m_map.tileState[i].x = 0.0f;
        m_map.tileState[i].y = 0.0f;
    }
    // Bulk update GPU buffer
    if (m_map.mapMesh.tileStateBuffer != 0)
    {
        glBindBuffer(GL_TEXTURE_BUFFER, m_map.mapMesh.tileStateBuffer);
        glBufferSubData(GL_TEXTURE_BUFFER, 0, m_map.tileState.size() * sizeof(glm::vec2), m_map.tileState.data());
        glBindBuffer(GL_TEXTURE_BUFFER, 0);
    }

    // Reset edit mode state
    m_lastHoverTile = UINT32_MAX;
    m_leftButtonPressed = false;
    m_dragActive = false;
    m_dragStartSelection.clear();

    m_lastHighlightMinX = 1;
    m_lastHighlightMaxX = 0;
    m_lastHighlightMinZ = 1;
    m_lastHighlightMaxZ = 0;

    m_mapState = _state;
}

void cMapSystem::updateSingleTileState(uint32_t tileIndex)
{
    if (!m_map.mapMesh.tileStateBuffer || tileIndex >= m_map.numTiles) return;
    glm::vec2 state = m_map.tileState[tileIndex];
    glBindBuffer(GL_TEXTURE_BUFFER, m_map.mapMesh.tileStateBuffer);
    glBufferSubData(GL_TEXTURE_BUFFER, tileIndex * sizeof(glm::vec2), sizeof(glm::vec2), &state);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
}

void cMapSystem::updateTileType(uint32_t tileIndex, eMapTileType newType)
{
    if (!m_map.mapMesh.tileTypeBuffer || tileIndex >= m_map.numTiles) return;
    uint32_t type = (uint32_t)newType;
    glBindBuffer(GL_TEXTURE_BUFFER, m_map.mapMesh.tileTypeBuffer);
    glBufferSubData(GL_TEXTURE_BUFFER, tileIndex * sizeof(uint32_t), sizeof(uint32_t), &type);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    m_map.tile[tileIndex].type = newType;
}

void cMapSystem::applyDragSelection(uint32_t startX, uint32_t startZ, uint32_t endX, uint32_t endZ, eDragMode mode)
{
    if (m_dragStartSelection.size() != m_map.numTiles)
        return;

    uint32_t width  = (uint32_t)m_map.dimensions.x;
    uint32_t height = (uint32_t)m_map.dimensions.y;
    startX = std::min(startX, width-1);
    startZ = std::min(startZ, height-1);
    endX   = std::min(endX,   width-1);
    endZ   = std::min(endZ,   height-1);

    uint32_t minX = std::min(startX, endX);
    uint32_t maxX = std::max(startX, endX);
    uint32_t minZ = std::min(startZ, endZ);
    uint32_t maxZ = std::max(startZ, endZ);

    for (uint32_t z = minZ; z <= maxZ; ++z)
    {
        for (uint32_t x = minX; x <= maxX; ++x)
        {
            uint32_t idx = x + z * width;
            float original = m_dragStartSelection[idx];
            float newVal = original;

            switch (mode)
            {
                case eDragMode::Replace:  newVal = 1.0f; break;
                case eDragMode::Add:      newVal = 1.0f; break;
                case eDragMode::Subtract: newVal = 0.0f; break;
            }

            if (newVal != m_map.tileState[idx].y)
            {
                m_map.tileState[idx].y = newVal;
                updateSingleTileState(idx);
            }
        }
    }

    if (mode == eDragMode::Replace)
    {
        for (uint32_t idx = 0; idx < m_map.numTiles; ++idx)
        {
            uint32_t x = idx % width;
            uint32_t z = idx / width;
            bool inside = (x >= minX && x <= maxX && z >= minZ && z <= maxZ);
            if (!inside)
            {
                float original = m_dragStartSelection[idx];
                if (original != 0.0f)
                {
                    m_map.tileState[idx].y = 0.0f;
                    updateSingleTileState(idx);
                }
            }
        }
    }
}

void cMapSystem::applyDragHighlight(uint32_t startX, uint32_t startZ,
                                    uint32_t endX, uint32_t endZ)
{
    uint32_t width = m_map.dimensions.x;
    uint32_t minX = std::min(startX, endX), maxX = std::max(startX, endX);
    uint32_t minZ = std::min(startZ, endZ), maxZ = std::max(startZ, endZ);

    // Clear previous highlight
    for (uint32_t z = m_lastHighlightMinZ; z <= m_lastHighlightMaxZ; ++z)
        for (uint32_t x = m_lastHighlightMinX; x <= m_lastHighlightMaxX; ++x)
        {
            uint32_t idx = x + z * width;
            if (m_map.tileState[idx].x != 0.0f)
            {
                m_map.tileState[idx].x = 0.0f;
                updateSingleTileState(idx);
            }
        }

    // Set new highlight
    for (uint32_t z = minZ; z <= maxZ; ++z)
        for (uint32_t x = minX; x <= maxX; ++x)
        {
            uint32_t idx = x + z * width;
            if (m_map.tileState[idx].x != 1.0f)
            {
                m_map.tileState[idx].x = 1.0f;
                updateSingleTileState(idx);
            }
        }

    m_lastHighlightMinX = minX; m_lastHighlightMaxX = maxX;
    m_lastHighlightMinZ = minZ; m_lastHighlightMaxZ = maxZ;
}

void cMapSystem::generateMapMesh(void)
{
    struct sMesh { std::vector<sMapMeshVertex> vertex; std::vector<std::uint32_t> index; } tempMesh;
    tempMesh.vertex.clear(); tempMesh.index.clear();
    if (m_map.numTiles == 0) return;

    const float yPos = 0.0f;

    const glm::vec2 uv00(0.0f, 0.0f);
    const glm::vec2 uv10(1.0f, 0.0f);
    const glm::vec2 uv11(1.0f, 1.0f);
    const glm::vec2 uv01(0.0f, 1.0f);

    for (std::uint32_t z = 0; z < (std::uint32_t)m_map.dimensions.y; ++z)
    {
        for (std::uint32_t x = 0; x < (std::uint32_t)m_map.dimensions.x; ++x)
        {
            glm::vec3 v0( x,      yPos,  z);
            glm::vec3 v1( x+1.0f, yPos,  z);
            glm::vec3 v2( x+1.0f, yPos,  z+1.0f);
            glm::vec3 v3( x,      yPos,  z+1.0f);

            glm::vec3 normal(0.0f, 1.0f, 0.0f);
            glm::vec4 tangent(1.0f, 0.0f, 0.0f, 1.0f);

            std::uint32_t base = (std::uint32_t)tempMesh.vertex.size();
            tempMesh.vertex.push_back({v0, normal, uv00, tangent});
            tempMesh.vertex.push_back({v1, normal, uv10, tangent});
            tempMesh.vertex.push_back({v2, normal, uv11, tangent});
            tempMesh.vertex.push_back({v3, normal, uv01, tangent});

            tempMesh.index.push_back(base+0); tempMesh.index.push_back(base+1); tempMesh.index.push_back(base+2);
            tempMesh.index.push_back(base+0); tempMesh.index.push_back(base+2); tempMesh.index.push_back(base+3);
        }
    }

    // Tangent generation
    std::vector<glm::vec3> tan1(tempMesh.vertex.size(), glm::vec3(0.0f));
    std::vector<glm::vec3> tan2(tempMesh.vertex.size(), glm::vec3(0.0f));
    for (size_t i = 0; i < tempMesh.index.size(); i += 3)
    {
        uint32_t i0 = tempMesh.index[i+0];
        uint32_t i1 = tempMesh.index[i+1];
        uint32_t i2 = tempMesh.index[i+2];
        const glm::vec3 &p0 = tempMesh.vertex[i0].position;
        const glm::vec3 &p1 = tempMesh.vertex[i1].position;
        const glm::vec3 &p2 = tempMesh.vertex[i2].position;
        const glm::vec2 &uv0 = tempMesh.vertex[i0].texCoord;
        const glm::vec2 &uv1 = tempMesh.vertex[i1].texCoord;
        const glm::vec2 &uv2 = tempMesh.vertex[i2].texCoord;
        glm::vec3 edge1 = p1 - p0;
        glm::vec3 edge2 = p2 - p0;
        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;
        float f = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
        if (fabs(f) < 1e-8f) continue;
        f = 1.0f / f;
        glm::vec3 tangent, bitangent;
        tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        tan1[i0] += tangent; tan1[i1] += tangent; tan1[i2] += tangent;
        tan2[i0] += bitangent; tan2[i1] += bitangent; tan2[i2] += bitangent;
    }
    for (size_t i = 0; i < tempMesh.vertex.size(); ++i)
    {
        const glm::vec3 &N = tempMesh.vertex[i].normal;
        const glm::vec3 &T = tan1[i];
        glm::vec3 tangent = glm::normalize(T - N * glm::dot(N, T));
        float w = (glm::dot(glm::cross(N, T), tan2[i]) < 0.0f) ? -1.0f : 1.0f;
        tempMesh.vertex[i].tangent = glm::vec4(tangent, w);
    }

    // Create VAO, VBO, EBO
    glGenVertexArrays(1, &m_map.mapMesh.VAO);
    glGenBuffers(1, &m_map.mapMesh.VBO);
    glGenBuffers(1, &m_map.mapMesh.EBO);

    glBindVertexArray(m_map.mapMesh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_map.mapMesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, tempMesh.vertex.size() * sizeof(sMapMeshVertex),
                 tempMesh.vertex.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_map.mapMesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, tempMesh.index.size() * sizeof(std::uint32_t),
                 tempMesh.index.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(sMapMeshVertex),
                          (void*)offsetof(sMapMeshVertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(sMapMeshVertex),
                          (void*)offsetof(sMapMeshVertex, normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(sMapMeshVertex),
                          (void*)offsetof(sMapMeshVertex, texCoord));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(sMapMeshVertex),
                          (void*)offsetof(sMapMeshVertex, tangent));
    glEnableVertexAttribArray(3);

    // TBO setup
    std::vector<std::uint32_t> tileTypes(m_map.numTiles);
    for (std::uint32_t i = 0; i < m_map.numTiles; ++i)
        tileTypes[i] = (std::uint32_t)m_map.tile[i].type;

    glGenBuffers(1, &m_map.mapMesh.tileTypeBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, m_map.mapMesh.tileTypeBuffer);
    glBufferData(GL_TEXTURE_BUFFER, tileTypes.size() * sizeof(std::uint32_t),
                 tileTypes.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &m_map.mapMesh.tileStateBuffer);
    glBindBuffer(GL_TEXTURE_BUFFER, m_map.mapMesh.tileStateBuffer);
    glBufferData(GL_TEXTURE_BUFFER, m_map.tileState.size() * sizeof(glm::vec2),
                 m_map.tileState.data(), GL_DYNAMIC_DRAW);

    glGenTextures(1, &m_map.mapMesh.tileTypeTBO);
    glBindTexture(GL_TEXTURE_BUFFER, m_map.mapMesh.tileTypeTBO);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, m_map.mapMesh.tileTypeBuffer);

    glGenTextures(1, &m_map.mapMesh.tileStateTBO);
    glBindTexture(GL_TEXTURE_BUFFER, m_map.mapMesh.tileStateTBO);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RG32F, m_map.mapMesh.tileStateBuffer);

    glBindVertexArray(0);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    m_map.mapMesh.numElement = (std::uint32_t)tempMesh.index.size();
}

bool cMapSystem::loadMap(const std::string& _fileName)
{
    freeMap();

    auto extractString = [](const std::string& line) -> std::string
    {
        size_t start = line.find('>');
        size_t end   = line.rfind('<');
        if (start == std::string::npos || end == std::string::npos || start + 1 >= end)
            return {};
        return line.substr(start + 1, end - start - 1);
    };

    auto extractVec2 = [&extractString](const std::string& line) -> glm::vec2
    {
        std::stringstream ss(extractString(line));
        float x, y;
        char comma;
        if (!(ss >> x >> comma >> y)) return {};
        return {x, y};
    };

    std::ifstream file(_fileName);
    if (!file.is_open())
    {
        std::cout << "Failed to load file: " << _fileName << std::endl;
        return false;
    }

    std::vector<std::vector<int>> tempTileRows;
    bool sizeProcessed = false;
    std::uint32_t currentRow = 0;
    std::string line;

    while (std::getline(file, line))
    {
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        size_t tag_start = line.find('<');
        size_t tag_end   = line.find('>');
        if (tag_start == std::string::npos || tag_end == std::string::npos)
            continue;

        std::string tag = line.substr(tag_start + 1, tag_end - tag_start - 1);

        if (tag == "map_name")
            m_map.name = extractString(line);
        else if (tag == "playerStartTile")
        {
            glm::vec2 pos = extractVec2(line);
            uint32_t x = static_cast<uint32_t>(pos.x);
            uint32_t z = static_cast<uint32_t>(pos.y);

            if (x < m_map.dimensions.x && z < m_map.dimensions.y)
                m_map.playerStartTile = x + z * static_cast<uint32_t>(m_map.dimensions.x);
        }
        else if (tag == "map_music")
            m_map.musicFile = extractString(line);
        else if (tag == "map_texture_atlas_diffuse")
        {
            std::string textureFile = extractString(line);
            m_map.textureAtlasDiffuse = m_graphicsSystem->loadTexture(FILE_PATH_TEXTURE + textureFile);
        }
        else if (tag == "map_texture_atlas_normal")
        {
            std::string textureFile = extractString(line);
            m_map.textureAtlasNormal = m_graphicsSystem->loadTexture(FILE_PATH_TEXTURE + textureFile);
        }
        else if (tag == "map_texture_atlas_specular")
        {
            std::string textureFile = extractString(line);
            m_map.textureAtlasSpecular = m_graphicsSystem->loadTexture(FILE_PATH_TEXTURE + textureFile);
        }
        else if (tag == "map_size")
        {
            m_map.dimensions = extractVec2(line);
            m_map.numTiles = static_cast<std::uint32_t>(m_map.dimensions.x * m_map.dimensions.y);
            m_map.tile.resize(m_map.numTiles);
            m_map.tileEntity.resize(m_map.numTiles, -1);
            m_map.tileEvent.resize(m_map.numTiles, eMapEventType::none);
            sizeProcessed = true;

            if (!tempTileRows.empty())
            {
                std::uint32_t row = 0;
                for (const auto& tileRow : tempTileRows)
                {
                    if (row >= static_cast<std::uint32_t>(m_map.dimensions.y))
                    {
                        std::cout << "Error: Too many tile rows" << std::endl;
                        return false;
                    }
                    if (tileRow.size() != static_cast<size_t>(m_map.dimensions.x))
                    {
                        std::cout << "Error: Tile row " << row << " has " << tileRow.size()
                                  << " tiles, expected " << m_map.dimensions.x << std::endl;
                        return false;
                    }
                    for (std::uint32_t col = 0; col < static_cast<std::uint32_t>(m_map.dimensions.x); ++col)
                    {
                        std::uint32_t idx = col + row * static_cast<std::uint32_t>(m_map.dimensions.x);
                        m_map.tile[idx].type = static_cast<eMapTileType>(tileRow[col]);
                    }
                    ++row;
                }
                currentRow = row;
                tempTileRows.clear();
            }
        }
        else if (tag == "map_tiles")
        {
            std::string content = extractString(line);
            if (content.empty())
                continue;

            std::vector<int> row;
            std::stringstream ss(content);
            std::string token;
            while (std::getline(ss, token, ','))
            {
                if (!token.empty())
                    row.push_back(std::stoi(token));
            }

            if (sizeProcessed)
            {
                if (currentRow >= static_cast<std::uint32_t>(m_map.dimensions.y))
                {
                    std::cout << "Error: Too many tile rows" << std::endl;
                    return false;
                }
                if (row.size() != static_cast<size_t>(m_map.dimensions.x))
                {
                    std::cout << "Error: Tile row " << currentRow << " has " << row.size()
                              << " tiles, expected " << m_map.dimensions.x << std::endl;
                    return false;
                }
                for (std::uint32_t col = 0; col < static_cast<std::uint32_t>(m_map.dimensions.x); ++col)
                {
                    std::uint32_t idx = col + currentRow * static_cast<std::uint32_t>(m_map.dimensions.x);
                    m_map.tile[idx].type = static_cast<eMapTileType>(row[col]);
                }
                ++currentRow;
            }
            else
            {
                tempTileRows.push_back(std::move(row));
            }
        }
        else if (tag == "entity_wall")
        {
            m_map.biomeEntities.wall = extractString(line);
        }
        else if (tag == "portal_tile")
        {
            glm::vec2 pos = extractVec2(line);
            uint32_t x = static_cast<uint32_t>(pos.x);
            uint32_t z = static_cast<uint32_t>(pos.y);

            if (x < m_map.dimensions.x && z < m_map.dimensions.y)
            {
                uint32_t idx = x + z * static_cast<uint32_t>(m_map.dimensions.x);
                m_map.tileEvent[idx] = eMapEventType::portal;
            }
        }
        else if (tag == "boss_alert_tile")
        {
            glm::vec2 pos = extractVec2(line);
            uint32_t x = static_cast<uint32_t>(pos.x);
            uint32_t z = static_cast<uint32_t>(pos.y);

            if (x < m_map.dimensions.x && z < m_map.dimensions.y)
            {
                uint32_t idx = x + z * static_cast<uint32_t>(m_map.dimensions.x);
                m_map.tileEvent[idx] = eMapEventType::bossAlert;
            }
        }
    }

    if (!sizeProcessed)
    {
        std::cout << "Error: Map size not specified" << std::endl;
        return false;
    }

    if (currentRow != static_cast<std::uint32_t>(m_map.dimensions.y))
    {
        std::cout << "Error: Expected " << m_map.dimensions.y << " tile rows, got " << currentRow << std::endl;
        return false;
    }

    m_map.tileState.resize(m_map.numTiles, glm::vec2(0.0f, 0.0f));
    generateMapMesh();

    // Instantiate wall entities for all wall tiles
    if (m_entitySystem && !m_map.biomeEntities.wall.empty())
    {
        const uint32_t width = m_map.dimensions.x;
        const uint32_t height = m_map.dimensions.y;
        for (uint32_t z = 0; z < height; ++z)
        {
            for (uint32_t x = 0; x < width; ++x)
            {
                const uint32_t idx = x + z * width;
                if (m_map.tile[idx].type == eMapTileType::wall)
                {
                    glm::vec3 pos(x + 0.5f, 0.0f, z + 0.5f);
                    int32_t id = m_entitySystem->loadEntity(FILE_PATH_ENTITY + m_map.biomeEntities.wall, pos);
                    if (id != -1)
                        m_map.tileEntity[idx] = id;
                }
            }
        }
    }

    lookAtTile(m_map.dimensions.x / 2, m_map.dimensions.y / 2);

    return true;
}

void cMapSystem::freeMap(void)
{
    if (m_map.mapMesh.VAO != 0) glDeleteVertexArrays(1, &m_map.mapMesh.VAO);
    if (m_map.mapMesh.VBO != 0) glDeleteBuffers(1, &m_map.mapMesh.VBO);
    if (m_map.mapMesh.EBO != 0) glDeleteBuffers(1, &m_map.mapMesh.EBO);
    if (m_map.mapMesh.tileTypeTBO != 0) glDeleteTextures(1, &m_map.mapMesh.tileTypeTBO);
    if (m_map.mapMesh.tileStateTBO != 0) glDeleteTextures(1, &m_map.mapMesh.tileStateTBO);
    if (m_map.mapMesh.tileTypeBuffer != 0) glDeleteBuffers(1, &m_map.mapMesh.tileTypeBuffer);
    if (m_map.mapMesh.tileStateBuffer != 0) glDeleteBuffers(1, &m_map.mapMesh.tileStateBuffer);
    m_map.mapMesh = {};

    if (m_map.textureAtlasDiffuse >= 0 && m_graphicsSystem)
        m_graphicsSystem->freeTexture(m_map.textureAtlasDiffuse);
    if (m_map.textureAtlasNormal >= 0 && m_graphicsSystem)
        m_graphicsSystem->freeTexture(m_map.textureAtlasNormal);
    if (m_map.textureAtlasSpecular >= 0 && m_graphicsSystem)
        m_graphicsSystem->freeTexture(m_map.textureAtlasSpecular);

    if (m_entitySystem)
    {
        for (int32_t id : m_map.tileEntity)
        {
            if (id != -1)
                m_entitySystem->destroyEntity(id);
        }
    }
    m_map.tileEntity.clear();

    m_map.name.clear();
    m_map.musicFile.clear();
    m_map.dimensions = glm::vec2(0.0f, 0.0f);
    m_map.numTiles = 0;
    m_map.tile.clear();
    m_map.textureAtlasDiffuse = -1;
    m_map.textureAtlasNormal = -1;
    m_map.textureAtlasSpecular = -1;

    sMapEvent* evt = nullptr;
    while ((evt = m_event.pop()) != nullptr)
        delete evt;
}

void cMapSystem::lookAtTile(std::uint32_t x, std::uint32_t z)
{
    if (!m_graphicsSystem) return;
    if (x >= static_cast<uint32_t>(m_map.dimensions.x) ||
        z >= static_cast<uint32_t>(m_map.dimensions.y))
        return;

    glm::vec3 tileCenter(x + 0.5f, 0.0f, z + 0.5f);
    glm::vec3 currentPos    = m_graphicsSystem->getCameraPosition();
    glm::vec3 currentLookAt = m_graphicsSystem->getCameraLookAt();
    glm::vec3 offset = currentPos - currentLookAt;
    glm::vec3 newPos = tileCenter + offset;
    m_graphicsSystem->setCameraPosition(newPos, tileCenter);
}

bool cMapSystem::getTileUnderMouse(double mouseX, double mouseY,
                                   uint32_t &outX, uint32_t &outZ,
                                   eMapTileType &outType)
{
    if (!m_graphicsSystem) return false;

    glm::mat4 view = m_graphicsSystem->getViewMatrix();
    glm::mat4 proj = m_graphicsSystem->getProjectionMatrix();
    int width  = m_graphicsSystem->getWindowWidth();
    int height = m_graphicsSystem->getWindowHeight();
    glm::vec3 cameraPos = m_graphicsSystem->getCameraPosition();

    float winX = static_cast<float>(mouseX);
    float winY = static_cast<float>(height) - static_cast<float>(mouseY);

    glm::vec3 winNear(winX, winY, 0.0f);
    glm::vec4 viewport(0.0f, 0.0f, width, height);
    glm::vec3 worldNear = glm::unProject(winNear, view, proj, viewport);
    glm::vec3 rayDir = glm::normalize(worldNear - cameraPos);

    if (fabs(rayDir.y) < 1e-6f) return false;
    float t = -cameraPos.y / rayDir.y;
    if (t <= 0.0f) return false;

    glm::vec3 intersection = cameraPos + t * rayDir;
    if (intersection.x < 0.0f || intersection.x >= m_map.dimensions.x ||
        intersection.z < 0.0f || intersection.z >= m_map.dimensions.y)
        return false;

    outX = static_cast<uint32_t>(floor(intersection.x));
    outZ = static_cast<uint32_t>(floor(intersection.z));
    uint32_t idx = outX + outZ * static_cast<uint32_t>(m_map.dimensions.x);
    outType = m_map.tile[idx].type;

    return true;
}

eMapEventType cMapSystem::getTileEvent(uint32_t _tileIndex) const
{
    if (_tileIndex < m_map.tileEvent.size())
        return m_map.tileEvent[_tileIndex];
    return eMapEventType::none;
}

bool cMapSystem::findPath(uint32_t startIdx, uint32_t goalIdx,
                          std::vector<uint32_t>& outPath) const
{
    if (startIdx == goalIdx) return false;
    uint32_t width = m_map.dimensions.x;
    uint32_t height = m_map.dimensions.y;
    if (startIdx >= width * height || goalIdx >= width * height) return false;

    auto heuristic = [width](uint32_t a, uint32_t b) -> uint32_t {
        uint32_t ax = a % width, az = a / width;
        uint32_t bx = b % width, bz = b / width;
        return (ax > bx ? ax - bx : bx - ax) + (az > bz ? az - bz : bz - az);
    };

    struct Node {
        uint32_t idx;
        uint32_t f;
        bool operator>(const Node& other) const { return f > other.f; }
    };

    std::priority_queue<Node, std::vector<Node>, std::greater<Node>> openSet;
    std::unordered_map<uint32_t, uint32_t> cameFrom;
    std::unordered_map<uint32_t, uint32_t> gScore;
    std::unordered_map<uint32_t, bool> inOpenSet;

    gScore[startIdx] = 0;
    openSet.push({startIdx, heuristic(startIdx, goalIdx)});
    inOpenSet[startIdx] = true;

    const int dx[] = {1, -1, 0, 0};
    const int dz[] = {0, 0, 1, -1};

    while (!openSet.empty())
    {
        Node current = openSet.top(); openSet.pop();
        inOpenSet[current.idx] = false;

        if (current.idx == goalIdx)
        {
            uint32_t crawl = goalIdx;
            while (crawl != startIdx)
            {
                outPath.push_back(crawl);
                crawl = cameFrom[crawl];
            }
            outPath.push_back(startIdx);
            std::reverse(outPath.begin(), outPath.end());
            return true;
        }

        uint32_t cx = current.idx % width;
        uint32_t cz = current.idx / width;
        for (int dir = 0; dir < 4; ++dir)
        {
            uint32_t nx = cx + dx[dir];
            uint32_t nz = cz + dz[dir];
            if (nx >= width || nz >= height) continue;
            uint32_t neighborIdx = nx + nz * width;

            if (m_map.tile[neighborIdx].type == eMapTileType::wall)
                continue;

            uint32_t tentativeG = gScore[current.idx] + 1;
            if (!gScore.count(neighborIdx) || tentativeG < gScore[neighborIdx])
            {
                cameFrom[neighborIdx] = current.idx;
                gScore[neighborIdx] = tentativeG;
                uint32_t f = tentativeG + heuristic(neighborIdx, goalIdx);
                if (!inOpenSet[neighborIdx])
                {
                    openSet.push({neighborIdx, f});
                    inOpenSet[neighborIdx] = true;
                }
            }
        }
    }
    return false;
}

void cMapSystem::setSelectedToType(const eMapTileType &_tileType)
{
    if (m_map.numTiles == 0) return;

    for (uint32_t i = 0; i < m_map.numTiles; ++i)
    {
        // Only process selected tiles (y == 1.0f)
        if (m_map.tileState[i].y != 1.0f)
            continue;

        eMapTileType oldType = m_map.tile[i].type;
        if (oldType == _tileType)
            continue;

        // Handle entity changes
        if (oldType == eMapTileType::wall)
        {
            if (m_entitySystem && m_map.tileEntity[i] != -1)
            {
                m_entitySystem->destroyEntity(m_map.tileEntity[i]);
                m_map.tileEntity[i] = -1;
            }
        }
        if (_tileType == eMapTileType::wall)
        {
            if (m_entitySystem && !m_map.biomeEntities.wall.empty())
            {
                uint32_t x = i % m_map.dimensions.x;
                uint32_t z = i / m_map.dimensions.x;
                glm::vec3 pos(x + 0.5f, 0.0f, z + 0.5f);
                int32_t id = m_entitySystem->loadEntity(FILE_PATH_ENTITY + m_map.biomeEntities.wall, pos);
                if (id != -1)
                    m_map.tileEntity[i] = id;
            }
        }

        m_map.tile[i].type = _tileType;
        updateTileType(i, _tileType);
    }
}

void cMapSystem::process(float _delta)
{
    if (m_mapState == eMapState::edit)
        processEdit(_delta);
    else if (m_mapState == eMapState::play)
        processPlay(_delta);
}

void cMapSystem::processEdit(float _delta)
{
    if (!m_graphicsSystem) return;

    bool shift = m_io->keyMap[GLFW_KEY_LEFT_SHIFT];
    bool ctrl  = m_io->keyMap[GLFW_KEY_LEFT_CONTROL];

    uint32_t tileX, tileZ;
    eMapTileType tileType;
    bool hit = getTileUnderMouse(m_io->mouseX, m_io->mouseY, tileX, tileZ, tileType);
    uint32_t hoverIndex = hit ? (tileX + tileZ * (uint32_t)m_map.dimensions.x) : UINT32_MAX;

    if (!m_dragActive && hoverIndex != m_lastHoverTile)
    {
        if (m_lastHoverTile < m_map.tileState.size())
        {
            m_map.tileState[m_lastHoverTile].x = 0.0f;
            updateSingleTileState(m_lastHoverTile);
        }
        if (hoverIndex < m_map.tileState.size())
        {
            m_map.tileState[hoverIndex].x = 1.0f;
            updateSingleTileState(hoverIndex);
        }
        m_lastHoverTile = hoverIndex;
    }

    bool leftDownNow = m_io->mouseLeftDown;
    bool leftReleasedNow = m_io->mouseLeftReleased;

    if (!m_leftButtonPressed && leftDownNow)
    {
        m_leftButtonPressed = true;
        m_dragActive = false;
        m_dragStartMousePos = glm::vec2(m_io->mouseX, m_io->mouseY);

        if (shift && !ctrl) m_dragMode = eDragMode::Add;
        else if (ctrl && !shift) m_dragMode = eDragMode::Subtract;
        else m_dragMode = eDragMode::Replace;

        m_dragStartSelection.resize(m_map.numTiles);
        for (uint32_t i = 0; i < m_map.numTiles; ++i)
            m_dragStartSelection[i] = m_map.tileState[i].y;

        if (hit)
        {
            m_dragStartTileX = tileX;
            m_dragStartTileZ = tileZ;
            m_dragEndTileX = tileX;
            m_dragEndTileZ = tileZ;
        }
    }

    if (m_leftButtonPressed && leftDownNow)
    {
        glm::vec2 currentMouse(m_io->mouseX, m_io->mouseY);
        float dx = currentMouse.x - m_dragStartMousePos.x;
        float dy = currentMouse.y - m_dragStartMousePos.y;

        if (!m_dragActive && dx*dx + dy*dy >= m_dragThresholdSq)
            m_dragActive = true;

        if (m_dragActive && hit)
        {
            if (tileX != m_dragEndTileX || tileZ != m_dragEndTileZ)
            {
                m_dragEndTileX = tileX;
                m_dragEndTileZ = tileZ;
                applyDragHighlight(m_dragStartTileX, m_dragStartTileZ,
                                   m_dragEndTileX, m_dragEndTileZ);
            }
        }
    }

    if (m_leftButtonPressed && leftReleasedNow)
    {
        if (m_dragActive)
        {
            applyDragSelection(m_dragStartTileX, m_dragStartTileZ,
                               m_dragEndTileX, m_dragEndTileZ, m_dragMode);
            for (uint32_t i = 0; i < m_map.numTiles; ++i)
                m_map.tileState[i].x = 0.0f;

            glBindBuffer(GL_TEXTURE_BUFFER, m_map.mapMesh.tileStateBuffer);
            glBufferSubData(GL_TEXTURE_BUFFER, 0,
                            m_map.tileState.size() * sizeof(glm::vec2),
                            m_map.tileState.data());
            glBindBuffer(GL_TEXTURE_BUFFER, 0);
        }
        else
        {
            if (hit)
            {
                uint32_t idx = tileX + tileZ * (uint32_t)m_map.dimensions.x;

                if (!shift && !ctrl)   // replace mode
                {
                    for (uint32_t i = 0; i < m_map.numTiles; ++i)
                        m_map.tileState[i].y = 0.0f;
                    m_map.tileState[idx].y = 1.0f;

                    glBindBuffer(GL_TEXTURE_BUFFER, m_map.mapMesh.tileStateBuffer);
                    glBufferSubData(GL_TEXTURE_BUFFER, 0,
                                    m_map.tileState.size() * sizeof(glm::vec2),
                                    m_map.tileState.data());
                    glBindBuffer(GL_TEXTURE_BUFFER, 0);
                }
                else
                {
                    float original = m_map.tileState[idx].y;
                    float newVal;
                    if (shift && !ctrl)      newVal = 1.0f;
                    else if (ctrl && !shift) newVal = 0.0f;
                    else                     newVal = (original == 0.0f) ? 1.0f : 0.0f;

                    if (newVal != original)
                    {
                        m_map.tileState[idx].y = newVal;
                        updateSingleTileState(idx);
                    }
                }
            }
        }

        m_leftButtonPressed = false;
        m_dragActive = false;
    }

    // Right mouse button: apply selected tile type to all selected tiles
    if (m_io->mouseRightPressed)
    {
        setSelectedToType(m_tileEditType);
    }
}

void cMapSystem::processPlay(float _delta)
{
    if (!m_graphicsSystem || !m_io) return;

    // Get the tile currently under the mouse cursor
    uint32_t tileX, tileZ;
    eMapTileType tileType;
    bool hit = getTileUnderMouse(m_io->mouseX, m_io->mouseY, tileX, tileZ, tileType);
    uint32_t hoverIndex = hit ? (tileX + tileZ * static_cast<uint32_t>(m_map.dimensions.x)) : UINT32_MAX;

    // Update highlight: clear previous, set new if valid
    if (hoverIndex != m_lastHoverTile)
    {
        // Clear highlight on previously hovered tile
        if (m_lastHoverTile < m_map.tileState.size())
        {
            m_map.tileState[m_lastHoverTile].x = 0.0f;
            updateSingleTileState(m_lastHoverTile);
        }

        // Set highlight on newly hovered tile
        if (hoverIndex < m_map.tileState.size())
        {
            m_map.tileState[hoverIndex].x = 1.0f;
            updateSingleTileState(hoverIndex);
        }

        m_lastHoverTile = hoverIndex;
    }

    // Handle tile click (left mouse button pressed)
    if (m_io->mouseLeftPressed && hit)
    {
        // Create a tile-clicked event
        sMapEvent* clickEvent = new sMapEvent();
        clickEvent->type = eMapEventType::tileClicked;
        clickEvent->data.tileX = tileX;
        clickEvent->data.tileZ = tileZ;
        clickEvent->data.tileIndex = hoverIndex;
        clickEvent->data.tileType = static_cast<std::uint32_t>(tileType);

        // Push to the event queue (assuming m_event is a thread‑safe queue)
        m_event.push(clickEvent);
    }

    // Optionally, we could also handle right‑click or other inputs here.
}

