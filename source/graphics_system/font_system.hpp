
#ifndef FONT_SYSTEM_HPP
#define FONT_SYSTEM_HPP

#include "galogen.hpp"
#include "stb_truetype.hpp"
#include "texture_component_define.hpp"

#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

class cFontSystem
{
public:
    bool initialize(const std::string &_fileName); // init with font file
    void terminate(void); // cleanup, free cached data, etc.


    // Render a text string to a texture and returns a sComponentTexture.
    // Returns -1 on failure.
    sComponentTexture renderStringToTexture(float _fontSize, const std::string& _text);

private:
    // cache font data
    std::vector<unsigned char> m_fontBuffer;         // holds the loaded font file
    stbtt_fontinfo             m_fontInfo;           // stbtt font info structure
    bool                       m_fontLoaded = false;
};

#endif // FONT_SYSTEM_HPP
