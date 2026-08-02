#ifndef GRAPHICS_SYSTEM_HPP
#define GRAPHICS_SYSTEM_HPP

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "galogen.hpp"
#include <GLFW/glfw3.h>

#include "../core/defines.hpp"
#include "../io_system/io_system_defines.hpp"
#include "../template/template_queue.hpp"
#include "../ui_system/ui_define.hpp"
#include "../map/map_define.hpp"
#include "../particle/particle_define.hpp"

#include "graphics_component_define.hpp"
#include "graphics_event_define.hpp"
#include "graphics_system_define.hpp"
#include "graphics_system_framebuffer.hpp"
#include "graphics_system_fsq.hpp"
#include "graphics_system_camera.hpp"
#include "graphics_system_shader.hpp"
#include "animation_system.hpp"
#include "model_system.hpp"
#include "texture_system.hpp"

class cGraphicsSystem
{
    public:
        // base interface
        bool initialize(void);
        void terminate(void);
        void process(float _delta);

        // event interface
        sGraphicsEvent* getEvent(void) { return m_event.pop(); }

        // External pointers
        void setIOPointer(std::shared_ptr<sIO> _io) { m_io = _io; }
        void setUIFormPointer(std::vector<sUIForm>* _uiForm) { m_uiForms = _uiForm; }
        void setUIColorPointer(sUIColor* _uiColor) { m_uiColor = _uiColor; }
        void setMapPointer(sMap* _map) { m_map = _map; }
        void setParticlePointer(std::vector<sParticle>* _particles) { m_particles = _particles; }
        void setParticleEmitterPointer(std::vector<sParticleEmitter>* _particleEmitters) { m_particleEmitters = _particleEmitters; }

        // component interface
        std::uint32_t getNewComponent(void);
        void destroyComponent(const std::int32_t &_index);
        void releaseComponent(const std::int32_t &_index);
        void updateComponentMatrix(const std::uint32_t &_index, const glm::vec3 &_position, const glm::vec3 &_direction);
        void setScale(const std::uint32_t &_index, const glm::vec3 &_scale);

        // window interface
        bool windowClosed(void) { return glfwWindowShouldClose(m_window); }
        void setWindow(std::uint32_t _resolutionX, std::uint32_t _resolutionY, bool _vsync, bool _fullscreen);

        // Animation interface
        void playAnimation(const std::int32_t &_gfxComponentIndex, const eAnimationType &_animation, bool _loop = true);
        void setAnimationIndex(const std::int32_t &_gfxComponentIndex, const eAnimationType &_animation, const std::int32_t &_index);

        // Resource interface
        void setParticleTexture(const std::string &_fileName);
        void setWinScreenTexture(const std::string &_fileName);
        void loadModel(const std::int32_t &_index, const std::string &_fileName);
        void setMeshName(const std::int32_t &_gfxComponentIndex, const std::uint32_t &_meshIndex, const std::string &_name);
        void setMeshEnabled(const std::int32_t &_gfxComponentIndex, const std::uint32_t &_meshIndex, const bool &_enabled);

        void loadTextureDiffuse(const std::int32_t &_index, const std::string &_fileName, const std::uint32_t _param = GL_CLAMP_TO_EDGE, std::uint32_t _meshIndex = 0);
        void loadTextureNormal(const std::int32_t &_index, const std::string &_fileName, const std::uint32_t _param = GL_CLAMP_TO_EDGE, std::uint32_t _meshIndex = 0);
        void loadTextureSpecular(const std::int32_t &_index, const std::string &_fileName, const std::uint32_t _param = GL_CLAMP_TO_EDGE, std::uint32_t _meshIndex = 0);

        bool         initializeFontSystem(const std::string &_fileName) { return m_textureSystem.initializeFontSystem(_fileName); }
        void         terminateFontSystem(void) { m_textureSystem.terminateFontSystem(); }
        std::int32_t generateTexture(const float &_fontSize, const std::string &_text) { return m_textureSystem.generateTexture(_fontSize, _text); }
        std::int32_t loadTexture(const std::string &_fileName, const std::uint32_t _param = GL_CLAMP_TO_EDGE) { return m_textureSystem.loadTexture(_fileName, _param); }
        void         freeTexture(const std::uint32_t _index) { return m_textureSystem.freeComponent(_index); }
        std::int32_t getTextureID(const std::int32_t _index) { return m_textureSystem.getTextureID(_index); }
        std::int32_t getTextureWidth(const std::int32_t _index) { return m_textureSystem.getTextureWidth(_index); }
        std::int32_t getTextureHeight(const std::int32_t _index) { return m_textureSystem.getTextureHeight(_index); }

        // Camera interface
        glm::vec3 getCameraPosition(void) { return m_camera.getPosition(); }
        glm::vec3 getCameraLookAt(void) { return m_camera.getLookAt(); }
        void setCameraPosition(const glm::vec3 &_position, const glm::vec3 &_lookAt) { return m_camera.setPosition(_position, _lookAt); }
        glm::mat4 getViewMatrix() const { return m_camera.getViewMatrix(); }
        glm::mat4 getProjectionMatrix() const { return m_camera.getProjectionMatrix(); }
        int getWindowWidth() const { return m_windowWidth; }
        int getWindowHeight() const { return m_windowHeight; }
        void updateZoom(void) { m_camera.updateZoom(glm::vec2(m_io->scrollX, m_io->scrollY)); }
        void moveCameraLeft(void) { m_camera.moveLeft(); }
        void moveCameraRight(void) { m_camera.moveRight(); }
        void moveCameraForward(void) { m_camera.moveForward(); }
        void moveCameraBackwards(void) { m_camera.moveBackwards(); }

        // Mode interface
        void setSystemState(const eSystemState &_systemState) { m_systemState = _systemState; }

    private:
        // graphics components
        std::vector<sComponentGraphics> m_components;
        std::vector<std::uint32_t>      m_freeList;
        void m_freeAllComponents(void);

        // Event
        tcQueue<sGraphicsEvent> m_event = {};

        // IO
        std::shared_ptr<sIO> m_io = {};

        // UI
        std::vector<sUIForm>* m_uiForms = {};
        sUIColor*             m_uiColor = {};

        // Map
        sMap* m_map = {};

        // State
       eSystemState m_systemState = eSystemState::normal;

        // Particles
        std::vector<sParticle>*        m_particles         = {};
        std::vector<sParticleEmitter>* m_particleEmitters  = {};

        // systems
        cAnimationSystem m_animationSystem = {};
        cModelSystem     m_modelSystem     = {};
        std::int32_t     m_loadModel(const std::string &_fileName) { return m_modelSystem.loadModel(_fileName); }
        cTextureSystem   m_textureSystem   = {};
        std::int32_t     m_loadTexture(const std::string &_fileName, const std::uint32_t _param = GL_CLAMP_TO_EDGE) { return m_textureSystem.loadTexture(_fileName, _param); }

        // camera
        cGraphicsSystemCamera m_camera = {};

        // scene map shader
        cGraphicsSystemShader m_sceneMapShader     = {};
        glm::mat4             m_mapmvp             = glm::mat4(1);
        std::uint32_t         m_mapmvp_loc         = 0;
        std::uint32_t         m_mapmodelMatrix_loc           = 0;
        std::uint32_t         m_mapsceneTexture_diffuse_Loc  = 0;
        std::uint32_t         m_mapsceneTexture_normal_Loc   = 0;
        std::uint32_t         m_mapsceneTexture_specular_Loc = 0;
        std::uint32_t         m_maplightDirLoc        = 0;
        std::uint32_t         m_maplightColorLoc      = 0;
        std::uint32_t         m_mapambientColorLoc    = 0;
        std::uint32_t         m_mapviewPosLoc         = 0;
        std::uint32_t         m_maptileTypeBufferLoc  = 0;
        std::uint32_t         m_maptileStateBufferLoc = 0;

        // scene shader
        cGraphicsSystemShader m_sceneShader     = {};
        glm::mat4             m_mvp             = glm::mat4(1);
        std::uint32_t         m_mvp_loc         = 0;
        std::uint32_t         m_modelMatrix_loc           = 0;
        std::uint32_t         m_sceneTexture_diffuse_Loc  = 0;
        std::uint32_t         m_sceneTexture_normal_Loc   = 0;
        std::uint32_t         m_sceneTexture_specular_Loc = 0;
        std::uint32_t         m_lightDirLoc     = 0;
        std::uint32_t         m_lightColorLoc   = 0;
        std::uint32_t         m_ambientColorLoc = 0;
        std::uint32_t         m_viewPosLoc      = 0;
        std::vector<GLint>    m_boneMatrixLocs  = {};

        // Particle rendering
        cGraphicsSystemShader m_particleShader  = {};
        GLuint                m_particleVAO     = 0;
        GLuint                m_particleVBO     = 0;
        std::uint32_t         m_particleTexLoc  = 0;
        std::int32_t          m_particleTexture = -1;   // default particle texture (white circle)

        // Win sceeen rendering
        cGraphicsSystemShader m_winShader    = {};
        std::uint32_t         m_winTexLoc    = 0;
        std::int32_t          m_winTextureID = -1;

        // ui shader
        cGraphicsSystemShader m_uiShader = {};
        std::uint32_t         m_modelLoc = 0;
        std::uint32_t         m_colorLoc = 0;
        std::uint32_t         m_uiTextureLoc = 0;
        std::uint32_t         m_useTextureLoc = 0;

        // post processing / compositing shader
        cGraphicsSystemShader m_postShader   = {};
        std::uint32_t         m_postModelLoc = 0;
        std::uint32_t         m_sceneTex_loc = 0;
        std::uint32_t         m_uiTex_loc    = 0;

        // framebuffer
        cGraphicsFramebuffer m_sceneFBO    = {};
        cGraphicsFramebuffer m_uiFBO       = {};

        // fullscreen quad NDC
        cGraphicsSystemFSQ   m_fsq = {};

        // window and context
        GLFWwindow*   m_window         = nullptr;
        GLFWmonitor*  m_primaryMonitor = nullptr;
        std::uint32_t m_windowWidth    = 800;
        std::uint32_t m_windowHeight   = 600;
        float         m_fieldOfView    = 45.0f;
        float         m_aspectRatio    = (float)m_windowWidth / (float)m_windowHeight;
        std::string   m_windowTitle    = "Fresh Meat";
        bool          m_vsyncEnabled   = true;
        bool          m_fullscreen     = false;

        // GLFW Callbacks
        static void m_scroll_callback(GLFWwindow* _window, double _xoffset, double _yoffset);
        static void m_cursor_position_callback(GLFWwindow* _window, double _xpos, double _ypos);
        static void m_mouse_button_callback(GLFWwindow* _window, int _button, int _action, int _mods);
        static void m_framebuffer_size_callback(GLFWwindow* _window, int _width, int _height);
        static void m_key_callback(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods);
        static void m_window_close_callback(GLFWwindow* _window);
};

#endif // GRAPHICS_SYSTEM_HPP
