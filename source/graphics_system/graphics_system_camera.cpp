
#include "graphics_system_camera.hpp"

void cGraphicsSystemCamera::initialize(const float &_fieldOfView, const std::uint32_t &_width, const std::uint32_t &_height, const float &_near, const float &_far)
{
    m_fieldOfView  = _fieldOfView;
    m_near         = _near;
    m_far          = _far;

    // setup projection
    setWindowSize(_width, _height);
}

void cGraphicsSystemCamera::updateZoom(const glm::vec2 &_scroll)
{
    m_position.y -= _scroll.y * m_scrollSpeed;
    m_lookAt.y -= _scroll.y * m_scrollSpeed;
    m_viewMatrix = glm::lookAt(m_position, m_lookAt, m_upVector);
}

void cGraphicsSystemCamera::moveLeft(void)
{
    glm::vec3 forward = glm::normalize(m_lookAt - m_position);
    glm::vec3 right = glm::normalize(glm::cross(forward, m_upVector));
    float step = 0.25f * m_scrollSpeed;   // consistent step size

    m_position -= right * step;
    m_lookAt   -= right * step;
    m_viewMatrix = glm::lookAt(m_position, m_lookAt, m_upVector);
}

void cGraphicsSystemCamera::moveRight(void)
{
    glm::vec3 forward = glm::normalize(m_lookAt - m_position);
    glm::vec3 right = glm::normalize(glm::cross(forward, m_upVector));
    float step = 0.25f * m_scrollSpeed;

    m_position += right * step;
    m_lookAt   += right * step;
    m_viewMatrix = glm::lookAt(m_position, m_lookAt, m_upVector);
}

void cGraphicsSystemCamera::moveForward(void)
{
    // Compute the forward direction (from camera position to look-at point)
    glm::vec3 forward = m_lookAt - m_position;

    // Project onto the XZ plane (ignore vertical component) to keep camera at constant height
    forward.y = 0.0f;

    // Normalize only if the vector has meaningful length
    if (glm::length(forward) > 0.001f)
        forward = glm::normalize(forward);
    else
        return;  // No horizontal movement possible (camera looking straight down/up)

    float step = 0.25f * m_scrollSpeed;  // Same step size as left/right movement

    // Move both camera position and look-at point forward
    m_position += forward * step;
    m_lookAt   += forward * step;

    // Recalculate the view matrix
    m_viewMatrix = glm::lookAt(m_position, m_lookAt, m_upVector);
}

void cGraphicsSystemCamera::moveBackwards(void)
{
    // Compute the forward direction (from camera position to look-at point)
    glm::vec3 forward = m_lookAt - m_position;

    // Project onto the XZ plane
    forward.y = 0.0f;

    // Normalize if possible
    if (glm::length(forward) > 0.001f)
        forward = glm::normalize(forward);
    else
        return;

    float step = 0.25f * m_scrollSpeed;

    // Move both camera position and look-at point backward
    m_position -= forward * step;
    m_lookAt   -= forward * step;

    // Recalculate the view matrix
    m_viewMatrix = glm::lookAt(m_position, m_lookAt, m_upVector);
}

void cGraphicsSystemCamera::setPosition(const glm::vec3 &_position, const glm::vec3 &_lookAt)
{
    m_position = _position;
    m_lookAt   = _lookAt;

    m_viewMatrix = glm::lookAt(m_position, m_lookAt, m_upVector);
}

void cGraphicsSystemCamera::setWindowSize(const std::uint32_t &_width, const std::uint32_t &_height)
{
    m_windowWidth  = _width;
    m_windowHeight = _height;
    m_aspectRatio  = (float)m_windowWidth / (float)m_windowHeight;

    // perspective
    if (m_perspective == true)
        m_projectionMatrix = glm::perspective(glm::radians(m_fieldOfView), m_aspectRatio, m_near, m_far);

    // orthographic
    else
        m_projectionMatrix = glm::ortho(-(m_windowWidth / 2.0f), m_windowWidth / 2.0f, -(m_windowHeight / 2.0f), m_windowHeight / 2.0f, m_near, m_far);
}
