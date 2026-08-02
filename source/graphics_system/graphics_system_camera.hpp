
#ifndef GRAPHICS_SYSTEM_CAMERA_HPP
#define GRAPHICS_SYSTEM_CAMERA_HPP

#include <cstdint>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


class cGraphicsSystemCamera
{
    public:
        void          initialize(const float &_fieldOfView, const std::uint32_t &_width, const std::uint32_t &_height, const float &_near, const float &_far);

        // interface
        void updateZoom(const glm::vec2 &_scroll);
        void moveLeft(void);
        void moveRight(void);
        void moveForward(void);
        void moveBackwards(void);

        // setters
        void          setPerspective(const bool &_perspective) { m_perspective = _perspective; };
        void          setPosition(const glm::vec3 &_position, const glm::vec3 &_lookAt);
        void          setWindowSize(const std::uint32_t &_width, const std::uint32_t &_height);

        // getters
        glm::vec3     getPosition(void) const { return m_position; }
        glm::vec3     getLookAt(void) const { return m_lookAt; }
        glm::mat4     getViewMatrix(void) const { return m_viewMatrix; }
        glm::mat4     getProjectionMatrix(void) const { return m_projectionMatrix; }

    protected:

    private:
        float         m_scrollSpeed      = 0.25f;
        bool          m_perspective      = true;
        float         m_fieldOfView      = 90.0f;
        float         m_near             = 0.1f;
        float         m_far              = 100.0f;
        std::uint32_t m_windowWidth      = 800;
        std::uint32_t m_windowHeight     = 600;
        float         m_aspectRatio      = 1.34; // 800.0f / 600.0f;
        glm::vec3     m_upVector         = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3     m_position         = glm::vec3(15.0f, 10.0f, 15.0f);
        glm::vec3     m_lookAt           = glm::vec3(5.0f, 0.0f, 5.0f);
        glm::mat4     m_viewMatrix       = glm::mat4(1);
        glm::mat4     m_projectionMatrix = glm::mat4(1);
};

#endif //GRAPHICS_SYSTEM_CAMERA_HPP


