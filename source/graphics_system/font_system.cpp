
#include "font_system.hpp"

bool cFontSystem::initialize(const std::string &_fileName) // init with font file
{
    // Load font file into memory
    FILE* file = fopen(_fileName.c_str(), "rb");
    if (!file)
        return false;

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    m_fontBuffer.resize(size);
    if (fread(m_fontBuffer.data(), 1, size, file) != static_cast<size_t>(size))
    {
        fclose(file);
        return false;
    }
    fclose(file);

    // Initialize stbtt_fontinfo
    if (!stbtt_InitFont(&m_fontInfo, m_fontBuffer.data(), 0))
    {
        m_fontBuffer.clear();
        return false;
    }

    m_fontLoaded = true;
    return true;
}

void cFontSystem::terminate(void) // cleanup, free cached data, etc.
{
    m_fontBuffer.clear();
    m_fontLoaded = false;
}

sComponentTexture cFontSystem::renderStringToTexture(float _fontSize, const std::string& _text)
{
    auto flipVertical = [](std::vector<unsigned char>& pixels, int width, int height, int bytesPerPixel = 4)
    {
        int rowSize = width * bytesPerPixel;
        std::vector<unsigned char> temp(rowSize);

        for (int y = 0; y < height / 2; ++y)
        {
            int topRow = y * rowSize;
            int bottomRow = (height - 1 - y) * rowSize;

            // swap rows
            memcpy(temp.data(), &pixels[topRow], rowSize);
            memcpy(&pixels[topRow], &pixels[bottomRow], rowSize);
            memcpy(&pixels[bottomRow], temp.data(), rowSize);
        }
    };

    sComponentTexture componentTexture = {};
    componentTexture.enabled = false;

    // 1. Ensure font data is loaded
    if (!m_fontLoaded)
        return componentTexture;

    // 2. Compute text metrics and required texture size
    float scale = stbtt_ScaleForPixelHeight(&m_fontInfo, _fontSize);
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&m_fontInfo, &ascent, &descent, &lineGap);
    int baseline = static_cast<int>(ascent * scale);

    int totalWidth = 0;
    for (char c : _text)
    {
        int advance, leftBearing;
        stbtt_GetCodepointHMetrics(&m_fontInfo, static_cast<int>(c), &advance, &leftBearing);
        totalWidth += static_cast<int>(advance * scale);
    }
    // Add some padding
    totalWidth += 2;
    int textureWidth = totalWidth;
    int textureHeight = static_cast<int>((ascent - descent + lineGap) * scale) + 2;

    // 3. Create a bitmap (RGBA) and draw each character
    std::vector<unsigned char> bitmap(textureWidth * textureHeight * 4, 0); // RGBA, initially transparent

    int xCursor = 1; // start with 1 pixel padding
    for (char c : _text)
    {
        int glyphIndex = stbtt_FindGlyphIndex(&m_fontInfo, static_cast<int>(c));
        if (glyphIndex == 0) continue; // character not present

        int width, height, xoff, yoff;
        unsigned char* glyphBitmap = stbtt_GetGlyphBitmap(&m_fontInfo, scale, scale, glyphIndex, &width, &height, &xoff, &yoff);

        if (glyphBitmap)
        {
            // Compute position on the bitmap (baseline + yoff)
            int startY = baseline + yoff;
            for (int y = 0; y < height; ++y)
            {
                int destY = startY + y;
                if (destY >= 0 && destY < textureHeight)
                {
                    for (int x = 0; x < width; ++x)
                    {
                        unsigned char alpha = glyphBitmap[y * width + x];
                        if (alpha > 0)
                        {
                            int destX = xCursor + xoff + x;
                            if (destX >= 0 && destX < textureWidth)
                            {
                                // Write RGBA (white color, alpha from glyph)
                                int idx = (destY * textureWidth + destX) * 4;
                                bitmap[idx + 0] = 255; // R
                                bitmap[idx + 1] = 255; // G
                                bitmap[idx + 2] = 255; // B
                                bitmap[idx + 3] = alpha;
                            }
                        }
                    }
                }
            }
            stbtt_FreeBitmap(glyphBitmap, nullptr);
        }

        // Advance cursor
        int advance;
        stbtt_GetGlyphHMetrics(&m_fontInfo, glyphIndex, &advance, nullptr);
        xCursor += static_cast<int>(advance * scale);
    }

    // 4. Create OpenGL texture from bitmap data
    int numChannels = 4; // RGBA
    GLuint texIndex = 0;

    // Validate input
    if (bitmap.empty() || textureWidth <= 0 || textureHeight <= 0)
        return componentTexture;

    // Determine OpenGL format
    GLenum format = GL_RGBA; // we always use RGBA

    // flip pixel data vertically
    flipVertical(bitmap, textureWidth, textureHeight);

    // Generate OpenGL texture
    glGenTextures(1, &texIndex);
    glBindTexture(GL_TEXTURE_2D, texIndex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, format, textureWidth, textureHeight, 0, format, GL_UNSIGNED_BYTE, bitmap.data());
    // Optionally generate mipmaps:
    // glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    // 5. Fill component texture struct
    componentTexture.enabled = true;
    componentTexture.fileName = _text; // store the rendered text as identifier
    componentTexture.ID = texIndex;
    componentTexture.width = static_cast<std::uint32_t>(textureWidth);
    componentTexture.height = static_cast<std::uint32_t>(textureHeight);
    componentTexture.numChannel = static_cast<std::uint32_t>(numChannels);

    return componentTexture;
}
