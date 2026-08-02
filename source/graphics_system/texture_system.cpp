
#include "texture_system.hpp"

void cTextureSystem::m_freeComponents(void)
{
    // for each component
    for (std::uint32_t i = 0; i < m_components.size(); ++i)
    {
        if (m_components[i].ID != 0)
        {
            glDeleteTextures(1, &m_components[i].ID);
            m_components[i].ID = 0;
        }
    }

    // free component vectors
    m_components.clear();
    m_freeList.clear();
}

std::uint32_t cTextureSystem::m_getNewComponent(void)
{
    // reuse disabled component
    if (!m_freeList.empty())
    {
        std::uint32_t index = m_freeList.back();
        m_freeList.pop_back();

        m_components[index] = sComponentTexture{};
        m_components[index].enabled = true;

        return index;
    }

    // create new component
    m_components.emplace_back();
    m_components.back().enabled = true;

    return m_components.size() - 1;
}

void cTextureSystem::freeComponent(const std::uint32_t &_index)
{
    // invalid index
    if (_index >= m_components.size())
        return;

    // already destroyed
    if (!m_components[_index].enabled)
        return;

    // free data
    if (m_components[_index].ID != 0)
    {
        glDeleteTextures(1, &m_components[_index].ID);
        m_components[_index].ID = 0;
    }

    // disable
    m_components[_index].enabled = false;
    m_freeList.push_back(_index);
}

std::int32_t cTextureSystem::getTextureID(const std::int32_t _index)
{
    // invalid index
    if ((_index < 0) || (static_cast<std::uint32_t>(_index) >= m_components.size()))
        return -1;

    // not enabled
    if (!m_components[_index].enabled)
        return -1;

    return m_components[_index].ID;
}

bool cTextureSystem::initialize(void)
{
    return true;
}

void cTextureSystem::terminate(void)
{
    // free all components
    m_freeComponents();
}

void cTextureSystem::process(float _delta)
{

}

std::int32_t cTextureSystem::loadTexture(const std::string &_fileName, const std::uint32_t _param)
{

    // first see if the texture is already loaded, if so return its index
    for (std::uint32_t i = 0; i < m_components.size(); ++i)
    {
        if ((m_components[i].enabled) && (m_components[i].fileName == _fileName))
            return i;
    }

    // Load using STBI
    std::int32_t width      = 0;
    std::int32_t height     = 0;
    std::int32_t numChannel = 0;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(_fileName.c_str(), &width, &height, &numChannel, 0);

    // Failed to load texture, exit
    if (data == nullptr)
    {
        std::cout << "Failed to load texture: " << _fileName << std::endl;
        return -1;
    }

    // Texture is less that the smallest required by OpenGL
    if ((width < 64) || (height < 64))
    {
        std::cout << "Texture size is too small: " << _fileName << std::endl;
    }

    // Setup format enum based on the number of channels in the image
    GLenum format = 0;
    if (numChannel == 1)
        format = GL_RED;
    else if (numChannel == 2)
        format = GL_RG;
    else if (numChannel == 3)
        format = GL_RGB;
    else if (numChannel == 4)
        format = GL_RGBA;

    // else use m_getNewComponent()
    std::uint32_t index = m_getNewComponent();
    m_components[index].fileName = _fileName;

    // Setup the texture pointer
    m_components[index].width      = static_cast<std::uint32_t>(width);
    m_components[index].height     = static_cast<std::uint32_t>(height);
    m_components[index].numChannel = static_cast<std::uint32_t>(numChannel);
    glGenTextures(1, &m_components[index].ID);
    glBindTexture(GL_TEXTURE_2D, m_components[index].ID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _param);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    return index;
}

std::int32_t  cTextureSystem::generateTexture(const float &_fontSize, const std::string &_text)
{
    sComponentTexture textureComponent = m_fontSystem.renderStringToTexture(_fontSize, _text);

    if (!textureComponent.enabled)
        return -1;

    // else use m_getNewComponent()
    std::uint32_t index = m_getNewComponent();
    m_components[index].fileName   = _text;
    m_components[index].ID         = textureComponent.ID;
    m_components[index].width      = textureComponent.width;
    m_components[index].height     = textureComponent.height;
    m_components[index].numChannel = textureComponent.numChannel;

    return index;
}

