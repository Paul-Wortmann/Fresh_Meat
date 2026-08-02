
#ifndef TEXTURE_COMPONENT_DEFINE_HPP
#define TEXTURE_COMPONENT_DEFINE_HPP

#include <cstdint>
#include <string>

struct sComponentTexture
{
    // component management
    std::string    fileName  = "";
    bool           enabled   = false;

    // OpenGL texture ID
    std::uint32_t  ID        = 0;

    // Texture data
    std::uint32_t width      = 0;
    std::uint32_t height     = 0;
    std::uint32_t numChannel = 0;
};

#endif // TEXTURE_COMPONENT_DEFINE_HPP

