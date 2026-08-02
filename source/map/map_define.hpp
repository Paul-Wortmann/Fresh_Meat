
#ifndef MAP_DEFINE_HPP
#define MAP_DEFINE_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "map_event_define.hpp"

enum class eMapState : std::uint32_t
{
    play = 0,
    edit = 1
};

enum class eMapBiome : std::uint32_t
{
    plains      = 0,
    desert      = 1,
    forrest     = 2,
    tundra      = 3
};

enum class eMapTileType : std::uint32_t
{
    none        = 0,
    floor       = 1,
    wall        = 2,
    path        = 3
};

struct sMapTile
{

    eMapTileType type      = eMapTileType::none;
};

// temp variables
struct sMapMeshVertex
{
    glm::vec3     position         = {};
    glm::vec3     normal           = {};
    glm::vec2     texCoord         = {};
    glm::vec4     tangent          = {};
};

struct sMapMesh
{
    std::uint32_t VAO             = 0;
    std::uint32_t VBO             = 0;
    std::uint32_t EBO             = 0;
    std::uint32_t numElement      = 0;

    std::uint32_t tileTypeTBO     = 0; // texture buffer for types
    std::uint32_t tileStateTBO    = 0; // texture buffer for states
    std::uint32_t tileTypeBuffer  = 0; // buffer object holding uint[]
    std::uint32_t tileStateBuffer = 0; // buffer object holding vec2[]
};

struct sMapBiomeEntities
{
    std::string wall = {};
};

struct sMap
{
    std::string                name                 = {};
    std::string                musicFile            = {};
    glm::uvec2                 dimensions           = {}; // tiles x, y
    eMapBiome                  biome                = eMapBiome::plains;
    sMapBiomeEntities          biomeEntities        = {};
    std::uint32_t              numTiles             = {}; // number of tiles
    std::vector<sMapTile>      tile                 = {};
    std::vector<glm::vec2>     tileState            = {}; // x = highlighted (0/1), y = selected (0/1)
    sMapMesh                   mapMesh              = {};
    std::int32_t               textureAtlasDiffuse  = -1;
    std::int32_t               textureAtlasNormal   = -1;
    std::int32_t               textureAtlasSpecular = -1;
    std::uint32_t              playerStartTile      =  0; // default player spawn point
    std::vector<std::int32_t>  tileEntity           = {}; // entity IDs used by the map.
    std::vector<eMapEventType> tileEvent            = {};
};

#endif // MAP_DEFINE_HPP

