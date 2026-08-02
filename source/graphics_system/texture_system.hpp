
#ifndef TEXTURE_SYSTEM_HPP
#define TEXTURE_SYSTEM_HPP

#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "galogen.hpp"
#include "stb_image.hpp"
#include "texture_component_define.hpp"
#include "font_system.hpp"

class cTextureSystem
{
    public:
        // base interface
        bool          initialize(void);
        void          terminate(void);
        void          process(float _delta);

        // component interface
        void          freeComponent(const std::uint32_t &_index);
        std::int32_t  loadTexture(const std::string &_fileName, const std::uint32_t _param = GL_CLAMP_TO_EDGE);

        // Font system interface
        bool          initializeFontSystem(const std::string &_fileName) { return m_fontSystem.initialize(_fileName); }
        void          terminateFontSystem(void) { m_fontSystem.terminate(); }
        std::int32_t  generateTexture(const float &_fontSize, const std::string &_text);

        // resource interface
        std::int32_t  getTextureID(const std::int32_t _index);
        std::int32_t  getTextureWidth(const std::int32_t _index) {return m_components[_index].width; }
        std::int32_t  getTextureHeight(const std::int32_t _index) {return m_components[_index].height; }

    protected:

    private:
        // model components
        std::vector<sComponentTexture> m_components;
        std::vector<std::uint32_t> m_freeList;
        std::uint32_t m_getNewComponent(void);
        void m_freeComponents(void);

        // systems
        cFontSystem m_fontSystem = {};
};

#endif // TEXTURE_SYSTEM_HPP

