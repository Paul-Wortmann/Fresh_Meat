
#include "application.hpp"

bool cApplication::initialize(void)
{
    // Return value
    std::uint32_t returnValue = true;

    // Initialize configuration system
    returnValue = (returnValue == true) ? m_configSystem.initialize()    : returnValue;

    // Load configuration
    m_configSystem.loadConfig();

    // Apply configuration settings: graphics
    m_graphicsSystem.setWindow(m_configSystem.getResolutionX(), m_configSystem.getResolutionY(), m_configSystem.getVsync(), m_configSystem.getFullscreen());

    std::string fontFile = std::string(FILE_PATH_FONT) + "OpenSans-Regular.ttf";

    // Initialize systems
    returnValue = (returnValue == true) ? m_audioSystem.initialize()                      : returnValue;
    returnValue = (returnValue == true) ? m_entitySystem.initialize()                     : returnValue;
    returnValue = (returnValue == true) ? m_graphicsSystem.initialize()                   : returnValue;
    returnValue = (returnValue == true) ? m_graphicsSystem.initializeFontSystem(fontFile) : returnValue;
    returnValue = (returnValue == true) ? m_ioSystem.initialize()                         : returnValue;
    returnValue = (returnValue == true) ? m_physicsSystem.initialize()                    : returnValue;
    returnValue = (returnValue == true) ? m_uiSystem.initialize()                         : returnValue;
    returnValue = (returnValue == true) ? m_mapSystem.initialize()                        : returnValue;
    returnValue = (returnValue == true) ? m_npcSystem.initialize()                        : returnValue;
    returnValue = (returnValue == true) ? m_playerSystem.initialize()                     : returnValue;
    returnValue = (returnValue == true) ? m_particleSystem.initialize()                   : returnValue;

    // Apply configuration settings: audio
    m_audioSystem.setVolumeMaster(m_configSystem.getVolumeMaster());
    m_audioSystem.setVolumeMusic(m_configSystem.getVolumeMusic());
    m_audioSystem.setVolumeSound(m_configSystem.getVolumeSound());

    // Setup inter-system dependencies
    if (returnValue == true)
    {
        m_entitySystem.setAudioSystem(&m_audioSystem);
        m_entitySystem.setGraphicsSystem(&m_graphicsSystem);
        m_entitySystem.setPhysicsSystem(&m_physicsSystem);
        m_graphicsSystem.setIOPointer(m_ioSystem.getIOPointer());
        m_graphicsSystem.setUIFormPointer(&m_uiSystem.getUIForms());
        m_graphicsSystem.setUIColorPointer(&m_uiSystem.getUIColor());
        m_graphicsSystem.setMapPointer(m_mapSystem.getMap());
        m_graphicsSystem.setParticlePointer(&m_particleSystem.getParticles());
        m_graphicsSystem.setParticleEmitterPointer(&m_particleSystem.getParticleEmitters());
        m_physicsSystem.setEntitySystem(&m_entitySystem);
        m_physicsSystem.setGraphicsSystem(&m_graphicsSystem);
        m_uiSystem.setIOPointer(m_ioSystem.getIOPointer());
        m_uiSystem.setGraphicsSystem(&m_graphicsSystem);
        m_uiSystem.setAudioSystem(&m_audioSystem);
        m_mapSystem.setEntitySystem(&m_entitySystem);
        m_mapSystem.setGraphicsSystem(&m_graphicsSystem);
        m_mapSystem.setIOPointer(m_ioSystem.getIOPointer());
        m_playerSystem.setMapPointer(&m_mapSystem);
        m_playerSystem.setEntityPointer(&m_entitySystem);
        m_playerSystem.setPhysicsPointer(&m_physicsSystem);
        m_playerSystem.setGraphicsPointer(&m_graphicsSystem);
        m_npcSystem.setPlayerSystem(&m_playerSystem);
        m_npcSystem.setPhysicsPointer(&m_physicsSystem);
        m_npcSystem.setEntityPointer(&m_entitySystem);
        m_npcSystem.setMapPointer(&m_mapSystem);

    }

    // load default particle
    m_graphicsSystem.setParticleTexture(std::string(FILE_PATH_TEXTURE) + std::string("particle_default.png"));

    // load default winscreen
    m_graphicsSystem.setWinScreenTexture(std::string(FILE_PATH_TEXTURE) + std::string("winScreen.png"));

    // Return
    return returnValue;
}

void cApplication::terminate(void)
{
    // config
    m_configSystem.saveConfig();

    // terminate
    m_audioSystem.terminate();
    m_graphicsSystem.terminateFontSystem();
    m_graphicsSystem.terminate();
    m_ioSystem.terminate();
    m_physicsSystem.terminate();
    m_entitySystem.terminate();
    m_uiSystem.terminate();
    m_mapSystem.terminate();
    m_npcSystem.terminate();
    m_particleSystem.terminate();
    m_playerSystem.terminate();
}

bool cApplication::run(void)
{
    if (initialize() == false)
    {
        m_gameState = eGameState::terminate;
        terminate();
        return EXIT_FAILURE;
    }

    // Seed the random number generator with the current time
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // set game state process
    m_gameState = eGameState::process;
    m_gameMode = eGameMode::menu;

    // load ui
    m_uiSystem.loadUI("data/ui.txt");
    m_uiSystem.setSliderValue(2,  6, m_configSystem.getVolumeMaster());
    m_uiSystem.setSliderValue(2,  8, m_configSystem.getVolumeMusic());
    m_uiSystem.setSliderValue(2, 10, m_configSystem.getVolumeSound());
    m_uiSystem.setFormEnabled(0, true);
    m_uiSystem.setFormEnabled(1, false);
    m_uiSystem.setFormEnabled(2, false);
    m_uiSystem.setFormEnabled(3, false);

    // load map
    m_mapSystem.loadMap(std::string(FILE_PATH_MAP) + "map_001.txt");

    // load map music
    m_audioSystem.loadMusic(std::string(FILE_PATH_MUSIC) + m_mapSystem.getMapMusic());
    m_audioSystem.playMusic();

    // load start sound
    std::uint32_t fresh_meat = m_audioSystem.loadSound(std::string(FILE_PATH_SOUND) + "fresh_meat_008.ogg");

    // load entities
    int32_t playerIndex = m_entitySystem.loadEntity(std::string(FILE_PATH_ENTITY) + "human_male_001.txt");
    //int32_t playerIndex = m_entitySystem.loadEntity(std::string(FILE_PATH_ENTITY) + "player_001.txt");
    //int32_t humanMaleIndex = m_entitySystem.loadEntity(std::string(FILE_PATH_ENTITY) + "human_male_001.txt");
    int32_t butcherIndex = m_entitySystem.loadEntity(std::string(FILE_PATH_ENTITY) + "butcher_001.txt");
    m_entitySystem.loadEntity(std::string(FILE_PATH_ENTITY) + "barrel_001.txt");
    m_entitySystem.loadEntity(std::string(FILE_PATH_ENTITY) + "gate_001.txt", glm::vec3(0.5,0.0,9.5));

    m_graphicsSystem.playAnimation(m_entitySystem.getGraphicsIndex(butcherIndex), eAnimationType::walk, true);
    m_graphicsSystem.playAnimation(m_entitySystem.getGraphicsIndex(playerIndex), eAnimationType::walk, true);


    // The player and npcs should have been loaded by this point.
    // Lets process entity system events to add npcs and players to thier systems

    // process entity system events
    for (sEntityEvent* tEvent = m_entitySystem.getEvent(); tEvent != nullptr; tEvent = m_entitySystem.getEvent())
    {
        // Player loaded
        if (tEvent->type == eEntityEventType::playerLoaded)
        {
            if (tEvent->data > 0)
            {
                m_npcSystem.addPlayer(tEvent->data);
                m_playerSystem.addPlayer(tEvent->data);
            }
        }
        // NPC loaded
        if (tEvent->type == eEntityEventType::npcLoaded)
        {
            if (tEvent->data > 0)
            {
                m_npcSystem.addNPC(tEvent->data);
                m_playerSystem.addNPC(tEvent->data);
            }
        }

        // Delete event
        delete tEvent;
    }

    // Player
    std::uint32_t playerTile = m_mapSystem.getPlayerStartTile();
    m_playerSystem.setPlayerStartTile(playerIndex, playerTile);
    m_playerSystem.lookAtPlayer(playerIndex);

    glm::uvec2 dimension = m_mapSystem.getDimensions();

    // brains
    for (uint32_t i = 0; i < 10; ++i)
    {
        std::uint32_t x = std::rand() % dimension.x;
        std::uint32_t z = std::rand() % dimension.y;
        m_entitySystem.loadEntity(std::string(FILE_PATH_ENTITY) + "brain_001.txt", glm::vec3(x, 0, z));
    }
    // lungs
    for (uint32_t i = 0; i < 10; ++i)
    {
        std::uint32_t x = std::rand() % dimension.x;
        std::uint32_t z = std::rand() % dimension.y;
        m_entitySystem.loadEntity(std::string(FILE_PATH_ENTITY) + "lungs_001.txt", glm::vec3(x, 0, z));
    }
    // hearts
    for (uint32_t i = 0; i < 10; ++i)
    {
        std::uint32_t x = std::rand() % dimension.x;
        std::uint32_t z = std::rand() % dimension.y;
        m_entitySystem.loadEntity(std::string(FILE_PATH_ENTITY) + "heart_001.txt", glm::vec3(x, 0, z));
    }
/*
    // gibs 3
    for (uint32_t i = 0; i < 10; ++i)
    {
        std::uint32_t x = std::rand() % dimension.x;
        std::uint32_t z = std::rand() % dimension.y;
        m_entitySystem.loadEntity(std::string(FILE_PATH_ENTITY) + "gibs_003.txt", glm::vec3(x, 0, z));
    }
    // gibs 4
    for (uint32_t i = 0; i < 10; ++i)
    {
        std::uint32_t x = std::rand() % dimension.x;
        std::uint32_t z = std::rand() % dimension.y;
        m_entitySystem.loadEntity(std::string(FILE_PATH_ENTITY) + "gibs_004.txt", glm::vec3(x, 0, z));
    }
    // gibs 5
    for (uint32_t i = 0; i < 10; ++i)
    {
        std::uint32_t x = std::rand() % dimension.x;
        std::uint32_t z = std::rand() % dimension.y;
        m_entitySystem.loadEntity(std::string(FILE_PATH_ENTITY) + "gibs_005.txt", glm::vec3(x, 0, z));
    }
    // gibs 6
    for (uint32_t i = 0; i < 10; ++i)
    {
        std::uint32_t x = std::rand() % dimension.x;
        std::uint32_t z = std::rand() % dimension.y;
        m_entitySystem.loadEntity(std::string(FILE_PATH_ENTITY) + "gibs_006.txt", glm::vec3(x, 0, z));
    }
    // gibs 7
    for (uint32_t i = 0; i < 10; ++i)
    {
        std::uint32_t x = std::rand() % dimension.x;
        std::uint32_t z = std::rand() % dimension.y;
        m_entitySystem.loadEntity(std::string(FILE_PATH_ENTITY) + "gibs_007.txt", glm::vec3(x, 0, z));
    }
    // gibs 8
    for (uint32_t i = 0; i < 10; ++i)
    {
        std::uint32_t x = std::rand() % dimension.x;
        std::uint32_t z = std::rand() % dimension.y;
        m_entitySystem.loadEntity(std::string(FILE_PATH_ENTITY) + "gibs_008.txt", glm::vec3(x, 0, z));
    }
    // gibs 9
    for (uint32_t i = 0; i < 10; ++i)
    {
        std::uint32_t x = std::rand() % dimension.x;
        std::uint32_t z = std::rand() % dimension.y;
        m_entitySystem.loadEntity(std::string(FILE_PATH_ENTITY) + "gibs_009.txt", glm::vec3(x, 0, z));
    }
*/

/*
    // test particles
    sParticleEmitter fireEmitter;
    fireEmitter.position = m_playerSystem.getPosition() + glm::vec3(0.0,2.0,0.0);
    fireEmitter.spread = 0.5f;
    fireEmitter.minLife = 0.5f;
    fireEmitter.maxLife = 1.2f;
    fireEmitter.minSpeed = 2.0f;
    fireEmitter.maxSpeed = 5.0f;
    fireEmitter.startColor = {1, 0.5, 0, 1};
    fireEmitter.endColor = {1, 0, 0, 0};
    fireEmitter.startSize = 0.5f;
    fireEmitter.endSize = 0.0f;
    fireEmitter.spawnRate = 30.0f;  // 30 particles per second
*/
    sParticleEmitter bloodEmitter;
    bloodEmitter.position       = glm::vec3(5.0f, 2.0f, 5.0f);   // where blood spawns
    bloodEmitter.spread         = 6.283f;                           // cone spread (radians)
    bloodEmitter.minLife        = 0.5f;                           // seconds
    bloodEmitter.maxLife        = 1.5f;
    bloodEmitter.minSpeed       = 2.0f;                           // units per second
    bloodEmitter.maxSpeed       = 5.0f;
    bloodEmitter.gravity        = glm::vec3(0.0f, -15.0f, 0.0f);  // stronger gravity for blood drops
    bloodEmitter.startColor     = glm::vec4(0.8f, 0.1f, 0.1f, 1.0f); // bright red
    bloodEmitter.endColor       = glm::vec4(0.4f, 0.0f, 0.0f, 0.0f);  // dark red & fade out
    bloodEmitter.startSize      = 0.5f;
    bloodEmitter.endSize        = 0.125f;
    bloodEmitter.spawnRate      = 0.0f;                          // particles per second (continuous)

std::size_t bloodEmitterId = m_particleSystem.createEmitter(bloodEmitter);

    using Clock = std::chrono::steady_clock;
    m_previousTime = Clock::now();
    m_lastPerfPrintTime = m_previousTime;

    while (m_gameState == eGameState::process)
    {
        // --- 1. Calculate delta time ---
        Clock::time_point currentTime = Clock::now();
        float frameDelta = std::chrono::duration<float>(currentTime - m_previousTime).count();
        m_previousTime = currentTime;
        frameDelta = std::min(frameDelta, 0.25f);

        // --- 2. Fixed‑timestep logic updates ---
        m_accumulator += frameDelta;

        // ui process state
        bool UIprocessed = false;

        while (m_accumulator >= m_fixedTimestep)
        {
            m_ioSystem.process();

            // Update all non‑graphics systems
            m_audioSystem.process(m_fixedTimestep);
            m_entitySystem.process(m_fixedTimestep);
            UIprocessed = m_uiSystem.process(m_fixedTimestep);
            if (!UIprocessed && m_gameMode == eGameMode::play) m_physicsSystem.process(m_fixedTimestep);
            if (!UIprocessed && m_gameMode == eGameMode::play) m_playerSystem.process(m_fixedTimestep);
            if (!UIprocessed && m_gameMode == eGameMode::play) m_npcSystem.process(m_fixedTimestep);
            if (!UIprocessed) m_mapSystem.process(m_fixedTimestep);
            if (!UIprocessed) m_particleSystem.process(m_fixedTimestep);

            m_logicUpdateCount++;
            m_accumulator -= m_fixedTimestep;
        }

        // --- 3. Rendering (graphics) ---
        if (m_targetGraphicsFps > 0.0f)
        {
            static Clock::time_point lastRenderTime = Clock::now();
            float timeSinceLastRender = std::chrono::duration<float>(currentTime - lastRenderTime).count();

            if (timeSinceLastRender >= m_targetGraphicsDelta)
            {
                m_graphicsSystem.process(timeSinceLastRender);
                m_renderCount++;
                lastRenderTime = currentTime;
            }
        }
        else
        {
            // Unlimited / vsync‑controlled
            m_graphicsSystem.process(frameDelta);
            m_renderCount++;
        }

        // --- 4. Print performance info every second (if enabled) ---
        if (m_printPerformance)
        {
            using Clock = std::chrono::steady_clock;
            auto now = Clock::now();
            float elapsed = std::chrono::duration<float>(now - m_lastPerfPrintTime).count();

            if (elapsed >= 1.0f)   // print roughly every second
            {
                int fps = static_cast<int>(m_renderCount / elapsed);
                int ups = static_cast<int>(m_logicUpdateCount / elapsed);
                //std::cout << "FPS: " << fps << ", UPS: " << ups << std::endl;

                // Reset counters
                m_renderCount = 0;
                m_logicUpdateCount = 0;
                m_lastPerfPrintTime = now;
            }
        }

        // Optional: yield CPU to prevent 100% usage when idle
        // std::this_thread::yield();

        // process graphics system events
        for (sGraphicsEvent* tEvent = m_graphicsSystem.getEvent(); tEvent != nullptr; tEvent = m_graphicsSystem.getEvent())
        {
            // Window has been closed
            if ((tEvent->type == eGraphicsEventType::windowClosed) && (tEvent->data == 1))
            {
                // terminate game
                m_gameState = eGameState::terminate;
            }
            // Scroll event
            if ((tEvent->type == eGraphicsEventType::scroll) && (tEvent->data == 1))
            {
                if (UIprocessed)
                {
                    //uiScroll
                }
                else
                {
                    // map scroll
                    m_graphicsSystem.updateZoom();
                }
            }

            // Delete event
            delete tEvent;
        }

        // process player system events
        for (sPlayerEvent* tEvent = m_playerSystem.getEvent(); tEvent != nullptr; tEvent = m_playerSystem.getEvent())
        {
            // Player tile changed
            if (tEvent->type == ePlayerEventType::tileChange)
            {
                // Query the map system if this tile has an event attached.
                eMapEventType mapEvent = m_mapSystem.getTileEvent(tEvent->data);

                // portal event tile
                if (mapEvent == eMapEventType::portal)
                {
                    std::cout << "Portal tile!" << std::endl;
                    m_graphicsSystem.setSystemState(eSystemState::win);
                    m_gameMode = eGameMode::win;
                    m_audioSystem.playSound(fresh_meat);
                }
            }
            // Player position changed
            else if (tEvent->type == ePlayerEventType::positionChange)
            {
                m_playerSystem.lookAtPlayer(playerIndex);
            }

            // Delete event
            delete tEvent;
        }

        // process NPC system events
        for (sNPCEvent* tEvent = m_npcSystem.getEvent(); tEvent != nullptr; tEvent = m_npcSystem.getEvent())
        {
            // NPC position changed
            if (tEvent->type == eNPCEventType::positionChange)
            {
            }

            // Delete event
            delete tEvent;
        }

        // process physics system events
        for (sPhysicsEvent* tEvent = m_physicsSystem.getEvent(); tEvent != nullptr; tEvent = m_physicsSystem.getEvent())
        {
            if (tEvent->type == ePhysicsEventType::collision)
            {
                std::int32_t playerPhysicsIdx = m_entitySystem.getPhysicsIndex(playerIndex);
                if (tEvent->bodyA.componentID == playerPhysicsIdx || tEvent->bodyB.componentID == playerPhysicsIdx)
                {
                    // Player collided with a static object
                    m_particleSystem.setEmitterPosition(bloodEmitterId, m_playerSystem.getPosition(playerIndex) + glm::vec3(0.0, 1.5, 0.0));
                    m_particleSystem.spawnParticles(bloodEmitterId, 10);
                }
            }
            // Delete event
            delete tEvent;
        }

        // process map system events
        for (sMapEvent* tEvent = m_mapSystem.getEvent(); tEvent != nullptr; tEvent = m_mapSystem.getEvent())
        {
            // Player stepped on a portal tile
            if (tEvent->type == eMapEventType::tileClicked)
            {
                //std::cout << "Clicked tile!" << std::endl;
                m_playerSystem.setPlayerTarget(playerIndex, tEvent->data.tileIndex);
            }

            // Delete event
            delete tEvent;
        }

        // process entity system events
        for (sEntityEvent* tEvent = m_entitySystem.getEvent(); tEvent != nullptr; tEvent = m_entitySystem.getEvent())
        {
            // Player loaded
            if (tEvent->type == eEntityEventType::playerLoaded)
            {
                if (tEvent->data > 0)
                {
                    m_npcSystem.addPlayer(tEvent->data);
                    m_playerSystem.addPlayer(tEvent->data);
                }
            }
            // NPC loaded
            if (tEvent->type == eEntityEventType::npcLoaded)
            {
                if (tEvent->data > 0)
                {
                    m_npcSystem.addNPC(tEvent->data);
                    m_playerSystem.addNPC(tEvent->data);
                }
            }

            // Delete event
            delete tEvent;
        }

        // process ui system events
        for (sUIEvent* tEvent = m_uiSystem.getEvent(); tEvent != nullptr; tEvent = m_uiSystem.getEvent())
        {
            // A button has been clicked
            if (tEvent->type == eUIEventType::buttonClicked)
            {
                std::cout << "Button clicked! " << tEvent->form << "-" << tEvent->element << std::endl;

                // ------------- form 0 - main menu -------------

                // Play:
                if (tEvent->form == 0 && tEvent->element == 1)
                {
                    m_uiSystem.setFormEnabled(0, false);
                    m_uiSystem.setFormEnabled(1, false);
                    m_uiSystem.setFormEnabled(2, false);
                    m_uiSystem.setFormEnabled(3, false);
                    m_mapSystem.setProcessState(eMapState::play);
                    m_gameMode = eGameMode::play;
                    //m_mapSystem.stopPathing();
                    //m_playerSystem.setupPlayer(m_playerTile);
                    //m_playerSystem.lookAtPlayer();
                    m_audioSystem.playSound(fresh_meat);
                }
                // Edit:
                if (tEvent->form == 0 && tEvent->element == 2)
                {
                    m_uiSystem.setFormEnabled(0, false);
                    m_uiSystem.setFormEnabled(1, false);
                    m_uiSystem.setFormEnabled(2, false);
                    m_uiSystem.setFormEnabled(3, true);
                    m_mapSystem.setProcessState(eMapState::edit);
                    m_gameMode = eGameMode::edit;
                    //m_playerTile = m_mapSystem.getPlayerTile();
                    //m_mapSystem.stopPathing();
                }
                // Save:
                if (tEvent->form == 0 && tEvent->element == 3)
                    ;//m_gameState = eGameState::terminate;

                // Load:
                if (tEvent->form == 0 && tEvent->element == 4)
                    ;//m_gameState = eGameState::terminate;

                // Options:
                if (tEvent->form == 0 && tEvent->element == 5)
                {
                    m_uiSystem.setFormEnabled(0, false);
                    m_uiSystem.setFormEnabled(2, true);
                }
                // Credits:
                if (tEvent->form == 0 && tEvent->element == 6)
                {
                    m_uiSystem.setFormEnabled(0, false);
                    m_uiSystem.setFormEnabled(1, true);
                }
                // Quit: terminate game
                if (tEvent->form == 0 && tEvent->element == 7)
                    m_gameState = eGameState::terminate;

                // ------------- form 1 - credits ---------------
                // Back:
                if (tEvent->form == 1 && tEvent->element == 3)
                {
                    m_uiSystem.setFormEnabled(1, false);
                    m_uiSystem.setFormEnabled(0, true);
                }

                // ------------- form 2 - options ---------------
                // Back:
                if (tEvent->form == 2 && tEvent->element == 11)
                {
                    m_uiSystem.setFormEnabled(2, false);
                    m_uiSystem.setFormEnabled(0, true);
                }

                // ------------- form 3 - edit ------------------
                // Back:
                if (tEvent->form == 3 && tEvent->element == 3)
                {
                    m_uiSystem.setFormEnabled(3, false);
                    m_uiSystem.setFormEnabled(0, true);
                }

            }

            // A slider has been moved
            if (tEvent->type == eUIEventType::sliderValueChanged)
            {
                std::cout << "Slider moved! " << tEvent->form << "-" << tEvent->element << " -> " << tEvent->value << std::endl;

                // ------------- form 2 - options ---------------
                // Volume Master:
                if (tEvent->form == 2 && tEvent->element == 6)
                {
                    m_audioSystem.setVolumeMaster(tEvent->value);
                }
                // Volume Music:
                if (tEvent->form == 2 && tEvent->element == 8)
                {
                    m_audioSystem.setVolumeMusic(tEvent->value);
                }
                // Volume Sound:
                if (tEvent->form == 2 && tEvent->element == 10)
                {
                    m_audioSystem.setVolumeSound(tEvent->value);
                }
            }

            // A drop-down box option has been changed
            if (tEvent->type == eUIEventType::dropdownSelectionChanged)
            {
                //std::cout << "Drop-down box selection changed! " << tEvent->form << "-" << tEvent->element << " -> " << tEvent->value << std::endl;

                // Tile type:
                if (tEvent->form == 3 && tEvent->element == 3)
                {
                    std::cout << "selection: " << tEvent->selection << std::endl;
                    m_mapSystem.setTileEditType(static_cast<eMapTileType>(tEvent->selection));
                }
            }

            // Delete event
            delete tEvent;
        }

        // Process io
        // escape key pressed - main menu
        if (m_ioSystem.getKey(GLFW_KEY_ESCAPE) == true)
        {
            if (m_gameMode == eGameMode::win)
                m_gameState = eGameState::terminate;

            m_uiSystem.setFormEnabled(0, true);
            m_uiSystem.setFormEnabled(1, false);
            m_uiSystem.setFormEnabled(2, false);
            m_uiSystem.setFormEnabled(3, false);
            m_gameMode = eGameMode::menu;
        }
        // ui active
        if (UIprocessed)
        {

        }
        // map active
        else
        {
            if (m_gameMode == eGameMode::edit)
            {
                if (m_ioSystem.getKey(GLFW_KEY_W)) m_graphicsSystem.moveCameraForward();
                if (m_ioSystem.getKey(GLFW_KEY_S)) m_graphicsSystem.moveCameraBackwards();
                if (m_ioSystem.getKey(GLFW_KEY_A)) m_graphicsSystem.moveCameraLeft();
                if (m_ioSystem.getKey(GLFW_KEY_D)) m_graphicsSystem.moveCameraRight();
            }
        }
    }

    terminate();
    return EXIT_SUCCESS;
}
