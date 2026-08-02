#ifndef MAP_SYSTEM_HPP
#define MAP_SYSTEM_HPP

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>

#include "../template/template_queue.hpp"
#include "../graphics_system/graphics_system.hpp"
#include "../entity_system/entity_system.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "map_define.hpp"
#include "map_event_define.hpp"

class cMapSystem
{
    public:
        bool initialize(void);
        void terminate(void);
        void process(float _delta);

        // external pointers
        void setEntitySystem(cEntitySystem* _entitySystem)   { m_entitySystem = _entitySystem; }
        void setGraphicsSystem(cGraphicsSystem* _graphicsSystem) { m_graphicsSystem = _graphicsSystem; }
        void setIOPointer(std::shared_ptr<sIO> _io)          { m_io = _io; }

        // event interface
        sMapEvent* getEvent(void) { return m_event.pop(); }

        // map state & loading
        void          setProcessState(const eMapState &_state);
        bool          loadMap(const std::string& _fileName);
        void          freeMap(void);
        sMap*         getMap(void) { return &m_map; }
        glm::uvec2    getDimensions(void) const { return m_map.dimensions; }
        std::uint32_t getPlayerStartTile(void) { return m_map.playerStartTile; }
        std::string   getMapMusic(void) { return m_map.musicFile; }

        // camera
        void          lookAtTile(std::uint32_t x, std::uint32_t z);

        // tile queries
        bool          getTileUnderMouse(double mouseX, double mouseY, uint32_t &outX, uint32_t &outZ, eMapTileType &outType);
        eMapEventType getTileEvent(uint32_t _tileIndex) const;

        // pathfinding service
        bool          findPath(uint32_t startIdx, uint32_t goalIdx, std::vector<uint32_t>& outPath) const;

        // tile type conversion for selected tiles (edit mode)
        void          setSelectedToType(const eMapTileType &_tileType);
        void          setTileEditType(const eMapTileType &_type) { m_tileEditType = _type; }
        eMapTileType  getTileEditType(void) const { return m_tileEditType; }

    private:
        // event queue
        tcQueue<sMapEvent> m_event = {};

        // external systems
        cEntitySystem*       m_entitySystem   = nullptr;
        cGraphicsSystem*     m_graphicsSystem = nullptr;
        std::shared_ptr<sIO> m_io             = {};

        // map data
        sMap          m_map;
        eMapState     m_mapState = eMapState::edit; // default to edit

        // edit mode state
        eMapTileType  m_tileEditType = eMapTileType::floor;
        uint32_t      m_lastHoverTile = UINT32_MAX;

        // drag selection
        bool          m_leftButtonPressed = false;
        bool          m_dragActive = false;
        glm::vec2     m_dragStartMousePos;
        uint32_t      m_dragStartTileX = 0, m_dragStartTileZ = 0;
        uint32_t      m_dragEndTileX = 0, m_dragEndTileZ = 0;
        const float   m_dragThresholdSq = 25.0f;
        enum class    eDragMode { Replace, Add, Subtract };
        eDragMode     m_dragMode;
        std::vector<float> m_dragStartSelection;

        // highlight rectangle bounds
        std::uint32_t m_lastHighlightMinX = 1;
        std::uint32_t m_lastHighlightMaxX = 0;
        std::uint32_t m_lastHighlightMinZ = 1;
        std::uint32_t m_lastHighlightMaxZ = 0;

        // private functions
        void generateMapMesh(void);
        void updateSingleTileState(uint32_t tileIndex);
        void updateTileType(uint32_t tileIndex, eMapTileType newType);
        void applyDragSelection(uint32_t startX, uint32_t startZ, uint32_t endX, uint32_t endZ, eDragMode mode);
        void applyDragHighlight(uint32_t startX, uint32_t startZ, uint32_t endX, uint32_t endZ);
        void processEdit(float _delta);
        void processPlay(float _delta);
};

#endif // MAP_SYSTEM_HPP
