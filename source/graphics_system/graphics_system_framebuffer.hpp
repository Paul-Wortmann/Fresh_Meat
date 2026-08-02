
#ifndef GRAPHICS_SYSTEM_FRAMEBUFFER_HPP
#define GRAPHICS_SYSTEM_FRAMEBUFFER_HPP

#include <cstdint>
#include "galogen.hpp"

class cGraphicsFramebuffer
{
public:
    bool initialize(uint32_t _width, uint32_t _height);
    void terminate();

    void bind();
    void unbind();

    uint32_t getColorTexture() const { return m_colorTexture; }

private:
    uint32_t m_FBO          = 0;
    uint32_t m_colorTexture = 0;
    uint32_t m_RBO          = 0;
};

#endif // GRAPHICS_SYSTEM_FRAMEBUFFER_HPP
