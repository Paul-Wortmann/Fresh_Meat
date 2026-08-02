
#ifndef PLAYER_DEFINE_HPP
#define PLAYER_DEFINE_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <glm/glm.hpp>

struct sPlayer
{
    int32_t  entityIndex   = -1; // index in entity system
    int32_t  physicsIndex  = -1;
    int32_t  graphicsIndex = -1;

    uint32_t currentTile    = 0;
    uint32_t lastTile       = UINT32_MAX;
    uint32_t targetTile     = 0;

    std::vector<uint32_t> path;
    uint32_t              pathIndex = 0;
    glm::vec3             targetPosition;

    std::vector<glm::vec3> waypoints;   // world positions of waypoints
    size_t                 waypointIndex = 0;

    bool     active = true;
};

#endif // PLAYER_DEFINE_HPP

