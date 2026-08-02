
#ifndef APPLICATION_HPP
#define APPLICATION_HPP

#include <cstdint>
#include <chrono>
#include <thread>

#include "application_defines.hpp"
#include "audio_system/audio_system.hpp"
#include "config_system/config_system.hpp"
#include "core/defines.hpp"
#include "entity_system/entity_system.hpp"
#include "graphics_system/graphics_system.hpp"
#include "io_system/io_system.hpp"
#include "map/map_system.hpp"
#include "npc/npc_system.hpp"
#include "particle/particle_system.hpp"
#include "player/player_system.hpp"
#include "physics_system/physics_system.hpp"
#include "ui_system/ui_system.hpp"

class cApplication
{
    public:
        // base
        bool initialize(void);
        void terminate(void);
        bool run(void);

        // timer
        void setFixedTimestep(float _seconds);   // e.g., 1/60 for 60 updates per second
        void setGraphicsFps(float _fps);         // 0 = unlimited (vsync), >0 = cap
        void enablePerformanceOutput(bool _enable) { m_printPerformance = _enable; } // optionally toggle output

    private:
        // member variables
        eGameState    m_gameState  = eGameState::initialize;
        eGameMode     m_gameMode   = eGameMode::menu;
        std::uint32_t m_playerTile = 0;

        // Fixed timestep for logic
        float m_fixedTimestep = 1.0f / 60.0f;      // default 60 Hz
        float m_accumulator = 0.0f;                // accumulates leftover time

        // Graphics frame rate cap (0 = unlimited)
        float m_targetGraphicsFps = 0.0f;          // 0 means no cap
        float m_targetGraphicsDelta = 0.0f;        // 0 means no cap

        // High‑resolution clock
        std::chrono::steady_clock::time_point m_previousTime;

        // Helper to enforce the cap (if any)
        void renderWithCap(float actualDelta);

        // Performance counters
        int m_logicUpdateCount = 0;                 // number of fixed updates this second
        int m_renderCount = 0;                     // number of renders this second
        std::chrono::steady_clock::time_point m_lastPerfPrintTime; // last print time
        bool m_printPerformance = true;            // whether to print (default on)

        // systems
        cAudioSystem    m_audioSystem;
        cConfigSystem   m_configSystem;
        cEntitySystem   m_entitySystem;
        cGraphicsSystem m_graphicsSystem;
        cIOSystem       m_ioSystem;
        cMapSystem      m_mapSystem;
        cNPCSystem      m_npcSystem;
        cParticleSystem m_particleSystem;
        cPhysicsSystem  m_physicsSystem;
        cPlayerSystem   m_playerSystem;
        cUISystem       m_uiSystem;
};

#endif // APPLICATION_HPP

