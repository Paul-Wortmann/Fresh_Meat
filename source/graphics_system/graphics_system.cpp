#include "graphics_system.hpp"

// ----------------------------------------------------------------------------
// Callbacks
// ----------------------------------------------------------------------------
void cGraphicsSystem::m_scroll_callback(GLFWwindow* _window, double _xoffset, double _yoffset)
{
    cGraphicsSystem* pSystemEngine = static_cast<cGraphicsSystem*>(glfwGetWindowUserPointer(_window));
    if (pSystemEngine)
    {
        pSystemEngine->m_io->scrollX = _xoffset;
        pSystemEngine->m_io->scrollY = _yoffset;

        sGraphicsEvent* event = new sGraphicsEvent;
        event->type = eGraphicsEventType::scroll;
        event->data = 1;
        pSystemEngine->m_event.push(event);
    }
}

void cGraphicsSystem::m_cursor_position_callback(GLFWwindow* _window, double _xpos, double _ypos)
{
    cGraphicsSystem* pSystemEngine = static_cast<cGraphicsSystem*>(glfwGetWindowUserPointer(_window));
    if (pSystemEngine)
    {
        pSystemEngine->m_io->mouseX = _xpos;
        pSystemEngine->m_io->mouseY = _ypos;
    }
}

void cGraphicsSystem::m_mouse_button_callback(GLFWwindow* _window, int _button, int _action, int _mods)
{
    cGraphicsSystem* pSystemEngine = static_cast<cGraphicsSystem*>(glfwGetWindowUserPointer(_window));
    if (pSystemEngine && pSystemEngine->m_io)
    {
        if (_button == GLFW_MOUSE_BUTTON_LEFT)
            pSystemEngine->m_io->mouseLeftDown = (_action == GLFW_PRESS);
        else if (_button == GLFW_MOUSE_BUTTON_RIGHT)
            pSystemEngine->m_io->mouseRightDown = (_action == GLFW_PRESS);
    }
}

void cGraphicsSystem::m_key_callback(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods)
{
    if (_key < 0 || _key > GLFW_KEY_LAST) return;
    cGraphicsSystem* pSystemEngine = static_cast<cGraphicsSystem*>(glfwGetWindowUserPointer(_window));
    if (pSystemEngine)
    {
        if (_action == GLFW_PRESS)
            pSystemEngine->m_io->keyMap[_key] = true;
        else if (_action == GLFW_RELEASE)
        {
            pSystemEngine->m_io->keyMap[_key] = false;
            pSystemEngine->m_io->keyMapReady[_key] = true;
        }
    }
}

void cGraphicsSystem::m_framebuffer_size_callback(GLFWwindow* _window, int _width, int _height)
{
    cGraphicsSystem* pSystemEngine = static_cast<cGraphicsSystem*>(glfwGetWindowUserPointer(_window));
    if (pSystemEngine)
    {
        pSystemEngine->m_windowWidth  = _width;
        pSystemEngine->m_windowHeight = _height;
        if (_height < 1) _height = 1;
        if (_width  < 1) _width  = 1;
        pSystemEngine->m_aspectRatio = (float)_width / (float)_height;

        glViewport(0, 0, pSystemEngine->m_windowWidth, pSystemEngine->m_windowHeight);
        pSystemEngine->m_camera.setWindowSize(pSystemEngine->m_windowWidth, pSystemEngine->m_windowHeight);

        pSystemEngine->m_sceneFBO.terminate();
        pSystemEngine->m_uiFBO.terminate();
        pSystemEngine->m_sceneFBO.initialize(_width, _height);
        pSystemEngine->m_uiFBO.initialize(_width, _height);
    }
}

void cGraphicsSystem::m_window_close_callback(GLFWwindow* _window)
{
    cGraphicsSystem* pSystemEngine = static_cast<cGraphicsSystem*>(glfwGetWindowUserPointer(_window));
    if (pSystemEngine)
    {
        sGraphicsEvent* event = new sGraphicsEvent;
        event->type = eGraphicsEventType::windowClosed;
        event->data = 1;
        pSystemEngine->m_event.push(event);
    }
}

// ----------------------------------------------------------------------------
// Public API
// ----------------------------------------------------------------------------
void cGraphicsSystem::setWindow(std::uint32_t _resolutionX, std::uint32_t _resolutionY, bool _vsync, bool _fullscreen)
{
    m_windowWidth  = _resolutionX;
    m_windowHeight = _resolutionY;
    if (m_windowHeight < 1) m_windowHeight = 1;
    if (m_windowWidth  < 1) m_windowWidth  = 1;
    m_aspectRatio  = (float)m_windowWidth / (float)m_windowHeight;
    m_vsyncEnabled = _vsync;
    m_fullscreen   = _fullscreen;
}

std::uint32_t cGraphicsSystem::getNewComponent(void)
{
    if (!m_freeList.empty())
    {
        std::uint32_t index = m_freeList.back();
        m_freeList.pop_back();
        m_components[index] = sComponentGraphics{};
        m_components[index].enabled = true;
        return index;
    }
    m_components.emplace_back();
    m_components.back().enabled = true;
    return m_components.size() - 1;
}

void cGraphicsSystem::destroyComponent(const std::int32_t &_index)
{
    if (_index < 0 || _index >= m_components.size())
        return;
    if (!m_components[_index].enabled)
        return;

    // free model
    if (m_components[_index].model != -1)
        m_modelSystem.freeComponent(m_components[_index].model);
    m_components[_index].model = -1;

    // free all materials (each material holds texture IDs)
    for (auto& mat : m_components[_index].material)
    {
        if (mat.diffuse != -1)
            m_textureSystem.freeComponent(mat.diffuse);
        if (mat.normal != -1)
            m_textureSystem.freeComponent(mat.normal);
        if (mat.specular != -1)
            m_textureSystem.freeComponent(mat.specular);
    }
    m_components[_index].material.clear();

    m_components[_index].enabled = false;
    m_freeList.push_back(_index);
}

void cGraphicsSystem::releaseComponent(const std::int32_t &_index)
{
    if (_index < 0 || _index >= m_components.size())
        return;
    if (!m_components[_index].enabled)
        return;

    m_components[_index].model = -1;

    // free all materials (each material holds texture IDs)
    for (auto& mat : m_components[_index].material)
    {
        mat.diffuse = -1;
        mat.normal = -1;
        mat.specular = -1;
    }
    m_components[_index].material.clear();

    m_components[_index].enabled = false;
    m_freeList.push_back(_index);
}

void cGraphicsSystem::updateComponentMatrix(const std::uint32_t &_index, const glm::vec3 &_position, const glm::vec3 &_direction)
{
    glm::mat4 model = glm::translate(glm::mat4(1), _position);
    if (glm::length(_direction) > 0.001f)
    {
        glm::vec3 dir = glm::normalize(_direction);
        float yaw = atan2(dir.x, dir.z);
        model = glm::rotate(model, yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    }
    model = model * glm::scale(glm::mat4(1), m_components[_index].scale);
    m_components[_index].modelMatrix = model;
}

void cGraphicsSystem::setScale(const std::uint32_t &_index, const glm::vec3 &_scale)
{
    assert(_index < m_components.size());
    assert(m_components[_index].enabled);
    m_components[_index].scale = _scale;
}

// ----------------------------------------------------------------------------
// Model and texture loading (updated for per‑mesh materials)
// ----------------------------------------------------------------------------
void cGraphicsSystem::loadModel(const std::int32_t &_index, const std::string &_fileName)
{
    if (_index < 0 || static_cast<std::uint32_t>(_index) >= m_components.size()) return;
    if (!m_components[_index].enabled) return;

    // load model (internally stores mesh data and returns model handle)
    m_components[_index].model = m_loadModel(_fileName);

    if (m_components[_index].model  < 0)
    {
        std::cout << "Model load failed: " << _fileName << std::endl;
        return; // model load failed
    }

    // resize material vector to match number of meshes
    int meshCount = m_modelSystem.getMeshCount(m_components[_index].model);
    m_components[_index].material.resize(meshCount);
}

void cGraphicsSystem::loadTextureDiffuse(const std::int32_t &_index, const std::string &_fileName, const std::uint32_t _param, std::uint32_t _meshIndex)
{
    if (_index < 0 || static_cast<std::uint32_t>(_index) >= m_components.size()) return;
    if (!m_components[_index].enabled) return;
    if (_meshIndex >= m_components[_index].material.size()) return;

    m_components[_index].material[_meshIndex].diffuse = m_loadTexture(_fileName, _param);
}

void cGraphicsSystem::loadTextureNormal(const std::int32_t &_index, const std::string &_fileName, const std::uint32_t _param, std::uint32_t _meshIndex)
{
    if (_index < 0 || static_cast<std::uint32_t>(_index) >= m_components.size()) return;
    if (!m_components[_index].enabled) return;
    if (_meshIndex >= m_components[_index].material.size()) return;

    m_components[_index].material[_meshIndex].normal = m_loadTexture(_fileName, _param);
}

void cGraphicsSystem::loadTextureSpecular(const std::int32_t &_index, const std::string &_fileName, const std::uint32_t _param, std::uint32_t _meshIndex)
{
    if (_index < 0 || static_cast<std::uint32_t>(_index) >= m_components.size()) return;
    if (!m_components[_index].enabled) return;
    if (_meshIndex >= m_components[_index].material.size()) return;

    m_components[_index].material[_meshIndex].specular = m_loadTexture(_fileName, _param);
}

void cGraphicsSystem::setParticleTexture(const std::string &_fileName)
{
    if (m_particleTexture != -1)
        m_textureSystem.freeComponent(m_particleTexture);
    m_particleTexture = m_loadTexture(_fileName, GL_CLAMP_TO_EDGE);
    std::cout << "Particle texture ID: " << m_particleTexture << std::endl;
}

void cGraphicsSystem::setWinScreenTexture(const std::string &_fileName)
{
    if (m_winTextureID != -1)
        m_textureSystem.freeComponent(m_winTextureID);
    m_winTextureID = m_loadTexture(_fileName, GL_CLAMP_TO_EDGE);
std::cout << "WinScreen texture ID: " << m_winTextureID << std::endl;
}

// ----------------------------------------------------------------------------
// Internal cleanup
// ----------------------------------------------------------------------------
void cGraphicsSystem::m_freeAllComponents(void)
{
    for (std::uint32_t i = 0; i < m_components.size(); ++i)
    {
        if (m_components[i].model != -1)
            m_modelSystem.freeComponent(m_components[i].model);
        m_components[i].model = -1;

        for (auto& mat : m_components[i].material)
        {
            if (mat.diffuse != -1)
                m_textureSystem.freeComponent(mat.diffuse);
            if (mat.normal != -1)
                m_textureSystem.freeComponent(mat.normal);
            if (mat.specular != -1)
                m_textureSystem.freeComponent(mat.specular);
        }
        m_components[i].material.clear();
    }
    m_components.clear();
    m_freeList.clear();
}

void cGraphicsSystem::playAnimation(const std::int32_t &_gfxComponentIndex, const eAnimationType &_animation, bool _loop)
{
    if (_gfxComponentIndex < 0 || (size_t)_gfxComponentIndex >= m_components.size()) return;
    if (!m_components[_gfxComponentIndex].enabled) return;
    if (m_components[_gfxComponentIndex].model < 0) return;

    // get index
    std::int32_t index = m_modelSystem.getAnimationIndex(m_components[_gfxComponentIndex].model, _animation);

    // no valid index found
    if (index < 0)
        return;

    m_animationSystem.playAnimation(m_components[_gfxComponentIndex].model, index, _loop);
}

void cGraphicsSystem::setAnimationIndex(const std::int32_t &_gfxComponentIndex, const eAnimationType &_animation, const std::int32_t &_index)
{
    if (_gfxComponentIndex < 0 || (size_t)_gfxComponentIndex >= m_components.size()) return;
    if (!m_components[_gfxComponentIndex].enabled) return;
    if (m_components[_gfxComponentIndex].model < 0) return;
    m_animationSystem.setAnimationIndex(m_components[_gfxComponentIndex].model, _animation, _index);
}

void cGraphicsSystem::setMeshName(const std::int32_t &_gfxComponentIndex, const std::uint32_t &_meshIndex, const std::string &_name)
{
    if (_gfxComponentIndex < 0 || (size_t)_gfxComponentIndex >= m_components.size()) return;
    if (!m_components[_gfxComponentIndex].enabled) return;
    if (m_components[_gfxComponentIndex].model < 0) return;
    m_modelSystem.setMeshName(m_components[_gfxComponentIndex].model, _meshIndex, _name);
}

void cGraphicsSystem::setMeshEnabled(const std::int32_t &_gfxComponentIndex, const std::uint32_t &_meshIndex, const bool &_enabled)
{
    if (_gfxComponentIndex < 0 || (size_t)_gfxComponentIndex >= m_components.size()) return;
    if (!m_components[_gfxComponentIndex].enabled) return;
    if (m_components[_gfxComponentIndex].model < 0) return;
    m_modelSystem.setMeshEnabled(m_components[_gfxComponentIndex].model, _meshIndex, _enabled);
}

// ----------------------------------------------------------------------------
// Initialization / Termination
// ----------------------------------------------------------------------------
bool cGraphicsSystem::initialize(void)
{
    if (!m_animationSystem.initialize())
    {
        std::cout << "Failed to initialize animation system." << std::endl;
        return false;
    }
    if (!m_modelSystem.initialize())
    {
        std::cout << "Failed to initialize model system." << std::endl;
        return false;
    }
    if (!m_textureSystem.initialize())
    {
        std::cout << "Failed to initialize texture system." << std::endl;
        return false;
    }
    if (glfwInit() == GLFW_FALSE)
    {
        std::cout << "Failed to initialize GLFW." << std::endl;
        return false;
    }
    // set system pointers
    m_animationSystem.setModelSystem(&m_modelSystem);

    std::cout << "GLFW version: " << glfwGetVersionString() << std::endl;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (m_fullscreen)
    {
        m_primaryMonitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(m_primaryMonitor);
        m_windowWidth  = mode->width;
        m_windowHeight = mode->height;
    }

    m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, m_windowTitle.c_str(), m_primaryMonitor, nullptr);
    if (!m_window)
    {
        std::cout << "Failed to create a GLFW window." << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(m_vsyncEnabled ? 1 : 0);

    std::cout << "Galogen version: " << GALOGEN_API_VER_MAJ << "." << GALOGEN_API_VER_MIN
              << " " << GALOGEN_API_NAME << " " << GALOGEN_API_PROFILE << std::endl;

    glViewport(0, 0, m_windowWidth, m_windowHeight);
    glfwSetWindowUserPointer(m_window, this);

    glfwSetScrollCallback         (m_window, m_scroll_callback);
    glfwSetCursorPosCallback      (m_window, m_cursor_position_callback);
    glfwSetMouseButtonCallback    (m_window, m_mouse_button_callback);
    glfwSetFramebufferSizeCallback(m_window, m_framebuffer_size_callback);
    glfwSetKeyCallback            (m_window, m_key_callback);
    glfwSetWindowCloseCallback    (m_window, m_window_close_callback);

    glClearColor(0.482352941f, 0.278431373f, 0.078431373f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    m_camera.initialize(m_fieldOfView, m_windowWidth, m_windowHeight, 0.1f, 100.0f);
    m_camera.setPosition(glm::vec3(15.0f, 10.0f, 15.0f), glm::vec3(5.0f, 0.0f, 5.0f));

    // Scene Map shader
    m_sceneMapShader.initialize(std::string(FILE_PATH_SHADER) + "scene_map");
    m_mapmvp_loc                = m_sceneMapShader.getUniformLocation("MVP");
    m_mapmodelMatrix_loc        = m_sceneMapShader.getUniformLocation("model");
    m_mapsceneTexture_diffuse_Loc   = m_sceneMapShader.getUniformLocation("diffuseTexture");
    m_mapsceneTexture_normal_Loc    = m_sceneMapShader.getUniformLocation("normalTexture");
    m_mapsceneTexture_specular_Loc  = m_sceneMapShader.getUniformLocation("specularTexture");
    m_maplightDirLoc            = m_sceneMapShader.getUniformLocation("lightDir");
    m_maplightColorLoc          = m_sceneMapShader.getUniformLocation("lightColor");
    m_mapambientColorLoc        = m_sceneMapShader.getUniformLocation("ambientColor");
    m_mapviewPosLoc             = m_sceneMapShader.getUniformLocation("viewPos");
    m_maptileTypeBufferLoc      = m_sceneMapShader.getUniformLocation("tileTypeBuffer");
    m_maptileStateBufferLoc     = m_sceneMapShader.getUniformLocation("tileStateBuffer");

    // Scene component shader
    m_sceneShader.initialize(std::string(FILE_PATH_SHADER) + "scene");
    m_mvp_loc                   = m_sceneShader.getUniformLocation("MVP");
    m_modelMatrix_loc           = m_sceneShader.getUniformLocation("model");
    m_sceneTexture_diffuse_Loc  = m_sceneShader.getUniformLocation("diffuseTexture");
    m_sceneTexture_normal_Loc   = m_sceneShader.getUniformLocation("normalTexture");
    m_sceneTexture_specular_Loc = m_sceneShader.getUniformLocation("specularTexture");
    m_lightDirLoc               = m_sceneShader.getUniformLocation("lightDir");
    m_lightColorLoc             = m_sceneShader.getUniformLocation("lightColor");
    m_ambientColorLoc           = m_sceneShader.getUniformLocation("ambientColor");
    m_viewPosLoc                = m_sceneShader.getUniformLocation("viewPos");

    m_boneMatrixLocs.resize(MAX_BONES);
    for (int j = 0; j < MAX_BONES; ++j)
    {
        std::string name = "boneMatrices[" + std::to_string(j) + "]";
        m_boneMatrixLocs[j] = glGetUniformLocation(m_sceneShader.getID(), name.c_str());
    }

    // UI shader
    m_uiShader.initialize(std::string(FILE_PATH_SHADER) + "uiShader");
    m_modelLoc   = glGetUniformLocation(m_uiShader.getID(), "model");
    m_colorLoc   = glGetUniformLocation(m_uiShader.getID(), "color");
    m_uiTextureLoc = glGetUniformLocation(m_uiShader.getID(), "uiTexture");
    m_useTextureLoc = glGetUniformLocation(m_uiShader.getID(), "useTexture");

    // Post processing shader
    m_postShader.initialize(std::string(FILE_PATH_SHADER) + "postprocess");
    m_postModelLoc = glGetUniformLocation(m_postShader.getID(), "model");
    m_sceneTex_loc = glGetUniformLocation(m_postShader.getID(), "sceneTex");
    m_uiTex_loc    = glGetUniformLocation(m_postShader.getID(), "uiTex");

    // Particle shader
    m_particleShader.initialize(std::string(FILE_PATH_SHADER) + "particle");
    m_particleTexLoc = m_particleShader.getUniformLocation("particleTex");
    if (m_particleShader.getID() <= 0)
        std::cout << "Warning: particle shader not loaded" << std::endl;

        // winScreen shader
        m_winShader.initialize(std::string(FILE_PATH_SHADER) + "winScreen");
        m_winTexLoc = m_winShader.getUniformLocation("screenTex");

    // Create a simple quad (centered at origin, size 1x1)
    float quadVertices[] =
    {
        // positions   // texcoords
        -0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  1.0f, 0.0f,

        -0.5f,  0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  1.0f, 1.0f
    };
    glGenVertexArrays(1, &m_particleVAO);
    glGenBuffers(1, &m_particleVBO);
    glBindVertexArray(m_particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_particleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    // FBO init
    m_sceneFBO.initialize(m_windowWidth, m_windowHeight);
    m_uiFBO.initialize(m_windowWidth, m_windowHeight);
    m_fsq.initialize();

    return true;
}

void cGraphicsSystem::terminate(void)
{
    for (sGraphicsEvent* tEvent = getEvent(); tEvent != nullptr; tEvent = getEvent())
        delete tEvent;

    m_freeAllComponents();

    glUseProgram(0);
    m_animationSystem.terminate();
    m_modelSystem.terminate();
    m_textureSystem.terminate();
    m_sceneMapShader.terminate();
    m_sceneShader.terminate();
    m_uiShader.terminate();
    m_postShader.terminate();

    // particles
    glDeleteVertexArrays(1, &m_particleVAO);
    glDeleteBuffers(1, &m_particleVBO);
    m_particleShader.terminate();
    if (m_particleTexture != -1)
        m_textureSystem.freeComponent(m_particleTexture);

    if (m_winTextureID != -1)
        m_textureSystem.freeComponent(m_winTextureID);

    glfwDestroyWindow(m_window);
    glfwTerminate();
}

// ----------------------------------------------------------------------------
// Main rendering loop (updated to use per‑mesh materials)
// ----------------------------------------------------------------------------
void cGraphicsSystem::process(float _delta)
{
    // animation system
    m_animationSystem.update(_delta);

    // normal render state
    if (m_systemState == eSystemState::normal)
    {
        // ---------- Scene pass ----------
        m_sceneFBO.bind();
        glEnable(GL_CULL_FACE);
        glViewport(0, 0, m_windowWidth, m_windowHeight);
        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // --- Map rendering (unchanged) ---
        m_sceneMapShader.use();
        glm::vec3 lightDir = glm::normalize(glm::vec3(-0.5f, -1.0f, -0.3f));
        glUniform3fv(m_maplightDirLoc, 1, glm::value_ptr(lightDir));
        glUniform3fv(m_mapviewPosLoc, 1, glm::value_ptr(m_camera.getPosition()));
        glUniform3f(m_maplightColorLoc, 0.5f, 0.5f, 0.5f);
        glUniform3f(m_mapambientColorLoc, 0.2f, 0.2f, 0.2f);

        glm::mat4 pvMatrix = m_camera.getProjectionMatrix() * m_camera.getViewMatrix();
        glDisable(GL_CULL_FACE);
        if (m_map && m_map->numTiles > 0 && m_map->mapMesh.VAO > 0)
        {
            glm::mat4 modelMatrix(1.0f);
            glm::mat4 mvp = pvMatrix * modelMatrix;
            glUniformMatrix4fv(m_mapmvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));
            glUniformMatrix4fv(m_mapmodelMatrix_loc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

            // bind atlas textures
            if (m_map->textureAtlasDiffuse != -1)
            {
                glUniform1i(m_mapsceneTexture_diffuse_Loc, 0);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_textureSystem.getTextureID(m_map->textureAtlasDiffuse));
            }
            if (m_map->textureAtlasNormal != -1)
            {
                glUniform1i(m_mapsceneTexture_normal_Loc, 1);
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, m_textureSystem.getTextureID(m_map->textureAtlasNormal));
            }
            if (m_map->textureAtlasSpecular != -1)
            {
                glUniform1i(m_mapsceneTexture_specular_Loc, 2);
                glActiveTexture(GL_TEXTURE2);
                glBindTexture(GL_TEXTURE_2D, m_textureSystem.getTextureID(m_map->textureAtlasSpecular));
            }

            glUniform1i(m_maptileTypeBufferLoc, 3);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_BUFFER, m_map->mapMesh.tileTypeTBO);
            glUniform1i(m_maptileStateBufferLoc, 4);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_BUFFER, m_map->mapMesh.tileStateTBO);

            glBindVertexArray(m_map->mapMesh.VAO);
            glDrawElements(GL_TRIANGLES, m_map->mapMesh.numElement, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
        glEnable(GL_CULL_FACE);

        // --- Scene objects with per‑mesh materials ---
        m_sceneShader.use();
        glUniform3fv(m_lightDirLoc, 1, glm::value_ptr(lightDir));
        glUniform3fv(m_viewPosLoc, 1, glm::value_ptr(m_camera.getPosition()));
        glUniform3f(m_lightColorLoc, 0.5f, 0.5f, 0.5f);
        glUniform3f(m_ambientColorLoc, 0.2f, 0.2f, 0.2f);

        glm::mat4 identityMatrix(1.0f);

        for (std::uint32_t i = 0; i < m_components.size(); ++i)
        {
            if (!m_components[i].enabled) continue;
            if (m_components[i].model == -1) continue;

            glm::mat4 modelMatrix = m_components[i].modelMatrix;
            glm::mat4 mvp = pvMatrix * modelMatrix;
            glUniformMatrix4fv(m_mvp_loc, 1, GL_FALSE, glm::value_ptr(mvp));
            glUniformMatrix4fv(m_modelMatrix_loc, 1, GL_FALSE, glm::value_ptr(modelMatrix));

            int meshCount = m_modelSystem.getMeshCount(m_components[i].model);
            // Ensure material vector size matches mesh count (should be true after loadModel)
            if (m_components[i].material.size() != static_cast<size_t>(meshCount))
            {
                // Fallback: resize if mismatch (should not happen)
                m_components[i].material.resize(meshCount);
            }

            // Pre‑compute joint matrices for this model
            std::vector<glm::mat4> jointMatrices;
            int modelIdx = m_components[i].model;
            bool hasJoints = !m_modelSystem.getComponent(modelIdx).skin.joints.empty();
            bool hasSkinning = hasJoints && m_animationSystem.hasAnimation(modelIdx);
            if (hasSkinning)
            {
                jointMatrices = m_animationSystem.getJointMatrices(modelIdx);
            }

            for (int m = 0; m < meshCount; ++m)
            {
                if (!m_modelSystem.getMeshEnabled(m_components[i].model, m)) continue;

                // textures
                const auto& mat = m_components[i].material[m];
                if (mat.diffuse != -1)
                {
                    glUniform1i(m_sceneTexture_diffuse_Loc, 0);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, m_textureSystem.getTextureID(mat.diffuse));
                }
                if (mat.normal != -1)
                {
                    glUniform1i(m_sceneTexture_normal_Loc, 1);
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, m_textureSystem.getTextureID(mat.normal));
                }
                if (mat.specular != -1)
                {
                    glUniform1i(m_sceneTexture_specular_Loc, 2);
                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, m_textureSystem.getTextureID(mat.specular));
                }

                // animation
                if (hasSkinning)
                {
                    for (size_t j = 0; j < jointMatrices.size() && j < m_boneMatrixLocs.size(); ++j)
                        glUniformMatrix4fv(m_boneMatrixLocs[j], 1, GL_FALSE, glm::value_ptr(jointMatrices[j]));
                }
                else
                {
                    for (size_t j = 0; j < m_boneMatrixLocs.size(); ++j)
                        glUniformMatrix4fv(m_boneMatrixLocs[j], 1, GL_FALSE, glm::value_ptr(identityMatrix));
                }

                glBindVertexArray(m_modelSystem.getMeshVAO(m_components[i].model, m));
                glDrawElements(GL_TRIANGLES, m_modelSystem.getMeshElementCount(m_components[i].model, m), GL_UNSIGNED_INT, 0);
            }
        }

        // --- Particles ---
        if (m_particles && !m_particles->empty())
        {
            m_particleShader.use();

            // Set particle texture
            glUniform1i(m_particleTexLoc, 0);
            glActiveTexture(GL_TEXTURE0);
            if (m_particleTexture != -1)
                glBindTexture(GL_TEXTURE_2D, m_textureSystem.getTextureID(m_particleTexture));
            else
                glBindTexture(GL_TEXTURE_2D, 0); // white fallback if shader handles it

            // Get shader uniforms
            GLint locMVP       = m_particleShader.getUniformLocation("MVP");
            GLint locModel     = m_particleShader.getUniformLocation("model");
            GLint locColor     = m_particleShader.getUniformLocation("particleColor");

            // Camera matrices
            glm::mat4 view = m_camera.getViewMatrix();
            glm::mat4 proj = m_camera.getProjectionMatrix();

            // For billboarding: extract camera right and up vectors from view matrix
            glm::vec3 cameraRight = glm::vec3(view[0][0], view[1][0], view[2][0]);
            glm::vec3 cameraUp    = glm::vec3(view[0][1], view[1][1], view[2][1]);

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glDisable(GL_CULL_FACE);        // particles are double‑sided
            glDepthMask(GL_FALSE);          // allow transparency overdraw (optional)
            glEnable(GL_DEPTH_TEST);        // still test depth to hide behind objects

            for (const auto& p : *m_particles)
            {
                if (!p.enabled) continue;

                // Compute billboard model matrix
                // Position + scale, rotate to face camera
                glm::mat4 model = glm::translate(glm::mat4(1.0f), p.position);
                // Scale by size (uniform scale)
                model = glm::scale(model, glm::vec3(p.size, p.size, 1.0f));
                // Rotate to face camera: the quad's local Z is world Z, we want it to face camera.
                // Billboarding: construct a rotation matrix from camera right/up.
                glm::mat4 billboard(1.0f);
                billboard[0][0] = cameraRight.x;
                billboard[0][1] = cameraRight.y;
                billboard[0][2] = cameraRight.z;
                billboard[1][0] = cameraUp.x;
                billboard[1][1] = cameraUp.y;
                billboard[1][2] = cameraUp.z;
                // third column (local Z) can stay as (0,0,1) – but we don't need it.
                model = model * billboard;

                glm::mat4 mvp = proj * view * model;

                glUniformMatrix4fv(locMVP, 1, GL_FALSE, glm::value_ptr(mvp));
                glUniformMatrix4fv(locModel, 1, GL_FALSE, glm::value_ptr(model));
                glUniform4fv(locColor, 1, glm::value_ptr(p.color));

                glBindVertexArray(m_particleVAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);
            }

            glBindVertexArray(0);
            glDepthMask(GL_TRUE);
            glEnable(GL_CULL_FACE);
        }

        m_sceneFBO.unbind();

        // ---------- UI pass (unchanged) ----------
        m_uiFBO.bind();
        glViewport(0, 0, m_windowWidth, m_windowHeight);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        m_uiShader.use();

        auto drawUIQuad = [&](float qx, float qy, float qw, float qh, float qz, int32_t texID, glm::vec4 qColor)
        {
            float ndcW = (qw / m_windowWidth) * 1.0f;
            float ndcH = (qh / m_windowHeight) * 1.0f;
            float ndcX = (qx / m_windowWidth) * 2.0f - 1.0f + ndcW;
            float ndcY = 1.0f - (qy / m_windowHeight) * 2.0f - ndcH;
            glm::mat4 modelMatrix = glm::translate(glm::mat4(1), glm::vec3(ndcX, ndcY, qz));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(ndcW, ndcH, 1.0f));
            glUniformMatrix4fv(m_modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            glUniform4fv(m_colorLoc, 1, glm::value_ptr(qColor));
            glUniform1i(m_useTextureLoc, (texID != -1) ? 1 : 0);
            if (texID != -1)
            {
                glUniform1i(m_uiTextureLoc, 0);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, m_textureSystem.getTextureID(texID));
            }
            m_fsq.process(_delta);
        };

        std::vector<sUIForm*> sortedForms;
        for (auto& form : *m_uiForms)
            if (form.enabled) sortedForms.push_back(&form);
        std::sort(sortedForms.begin(), sortedForms.end(),
                  [](const sUIForm* a, const sUIForm* b) { return a->level < b->level; });

        // Pass 1: backgrounds & normal elements
        for (auto* formPtr : sortedForms)
        {
            auto& form = *formPtr;
            drawUIQuad(form.position.x, form.position.y, form.dimensions.x, form.dimensions.y,
                       form.level * 0.001f, form.textureID, m_uiColor->colorBackground);
            for (auto& el : form.elements)
            {
                if (!el.enabled) continue;
                float x = form.position.x + el.position.x;
                float y = form.position.y + el.position.y;
                float z = form.level * 0.001f;
                float w = el.dimensions.x;
                float h = el.dimensions.y;

                glm::vec4 color;
                if (el.type == eUIElementType::titleBar)
                    color = m_uiColor->colorDefault;
                else if (el.type == eUIElementType::progressBar)
                    color = m_uiColor->colorIlluminated;
                else if (el.type == eUIElementType::text)
                    color = m_uiColor->colorText;
                else
                {
                    switch (el.state)
                    {
                        case eUIElementState::hovered: color = m_uiColor->colorHovered; break;
                        case eUIElementState::pressed: color = m_uiColor->colorPressed; break;
                        case eUIElementState::clicked: color = m_uiColor->colorClicked; break;
                        default:                       color = m_uiColor->colorDefault; break;
                    }
                }

                if (el.type == eUIElementType::dropdown)
                {
                    if (el.textureBaseID != -1)
                        drawUIQuad(x, y, w, h, z, el.textureBaseID, color);
                    if (!el.options.empty() && el.selectedIndex >= 0 &&
                        el.selectedIndex < (int)el.optionTextures.size())
                        drawUIQuad(x, y, w, h, z + 0.0001f, el.optionTextures[el.selectedIndex], m_uiColor->colorText);
                }
                else if (el.type == eUIElementType::text)
                {
                    if (el.textTextureID != -1)
                        drawUIQuad(x, y, w, h, z + 0.0001f, el.textTextureID, color);
                }
                else if (el.type == eUIElementType::button)
                {
                    int32_t tex = (el.state == eUIElementState::hovered && el.textureExtID != -1) ? el.textureExtID : el.textureBaseID;
                    if (tex != -1)
                        drawUIQuad(x, y, w, h, z, tex, color);
                    if (el.textTextureID != -1)
                        drawUIQuad(x, y, w, h, z + 0.0001f, el.textTextureID, m_uiColor->colorText);
                }
                else if (el.type == eUIElementType::slider)
                {
                    if (el.textureBaseID != -1)
                        drawUIQuad(x, y, w, h, z, el.textureBaseID, color);
                    float t = el.elementValue / 100.0f;
                    float knobX = glm::clamp(x + t * w - el.dimensionsExt.x * 0.5f, x, x + w - el.dimensionsExt.x);
                    float knobY = y + (h - el.dimensionsExt.y) * 0.5f;
                    drawUIQuad(knobX, knobY, el.dimensionsExt.x, el.dimensionsExt.y, z + 0.0001f, el.textureExtID, color);
                }
                else if (el.type == eUIElementType::progressBar)
                {
                    if (el.textureBaseID != -1)
                        drawUIQuad(x, y, w, h, z, el.textureBaseID, color);
                    float fillW = w * (el.elementValue / 100.0f);
                    if (fillW > 0.0f)
                        drawUIQuad(x, y, fillW, h, z + 0.0001f, el.textureExtID, m_uiColor->colorIlluminated);
                }
                else if (el.type == eUIElementType::titleBar)
                {
                    if (el.textureBaseID != -1)
                        drawUIQuad(x, y, w, h, z, el.textureBaseID, color);
                    if (el.textTextureID != -1)
                        drawUIQuad(x, y, w, h, z + 0.0001f, el.textTextureID, m_uiColor->colorText);
                }
            }
        }

        // Pass 2: expanded dropdown options
        for (auto* formPtr : sortedForms)
        {
            auto& form = *formPtr;
            for (auto& el : form.elements)
            {
                if (!el.enabled || el.type != eUIElementType::dropdown || !el.expanded)
                    continue;
                float x = form.position.x + el.position.x;
                float y = form.position.y + el.position.y;
                float z = form.level * 0.001f;
                float w = el.dimensions.x;
                float h = el.dimensions.y;
                for (size_t i = 0; i < el.options.size(); ++i)
                {
                    float optY = y + h * (i + 1);
                    glm::vec4 optColor = (i == el.hoveredOptionIndex) ? m_uiColor->colorHovered : m_uiColor->colorDefault;
                    drawUIQuad(x, optY, w, h, z + 0.0002f, el.textureBaseID, optColor);
                    if (i < el.optionTextures.size())
                        drawUIQuad(x, optY, w, h, z + 0.0003f, el.optionTextures[i], m_uiColor->colorText);
                }
            }
        }

        m_uiFBO.unbind();

        // ---------- Composite pass ----------
        glClear(GL_COLOR_BUFFER_BIT);
        m_postShader.use();
        glUniform1i(m_sceneTex_loc, 0);
        glUniform1i(m_uiTex_loc, 1);
        glDisable(GL_DEPTH_TEST);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_sceneFBO.getColorTexture());
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, m_uiFBO.getColorTexture());
        m_fsq.process(_delta);
        glEnable(GL_DEPTH_TEST);

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }

    else if (m_systemState == eSystemState::win)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, m_windowWidth, m_windowHeight);

        glClearColor(0.f, 0.f, 0.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        m_winShader.use();

        glUniform1i(m_winTexLoc, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_textureSystem.getTextureID(m_winTextureID));

        m_fsq.process(_delta);

        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}
