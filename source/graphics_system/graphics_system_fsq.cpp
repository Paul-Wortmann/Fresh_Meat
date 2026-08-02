
#include "graphics_system_fsq.hpp"


bool cGraphicsSystemFSQ::initialize(void)
{
    struct sVert
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 textureCoord;
    };

    // Quad vertices in NDC with normals and texture coordinates
    std::vector<sVert> vertices = {
        { glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f) }, // bottom left
        { glm::vec3( 1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f) }, // bottom right
        { glm::vec3( 1.0f,  1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f) }, // top right
        { glm::vec3(-1.0f,  1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f) }  // top left
    };

    // Indices for two triangles forming the quad
    std::vector<std::uint32_t> indices = {
        0, 1, 2,   // first triangle
        0, 2, 3    // second triangle
    };
    m_numElements = static_cast<std::uint32_t>(indices.size());

    // Generate and bind VAO
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    // Generate and fill VBO
    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(sVert), vertices.data(), GL_STATIC_DRAW);

    // Generate and fill EBO
    glGenBuffers(1, &m_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(std::uint32_t), indices.data(), GL_STATIC_DRAW);

    // Set up vertex attributes
    // Position attribute (location 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(sVert), (void*)offsetof(sVert, position));
    glEnableVertexAttribArray(0);

    // Normal attribute (location 1)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(sVert), (void*)offsetof(sVert, normal));
    glEnableVertexAttribArray(1);

    // Texture coordinate attribute (location 2)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(sVert), (void*)offsetof(sVert, textureCoord));
    glEnableVertexAttribArray(2);

    // Unbind VAO
    glBindVertexArray(0);

    // Optional error check
    if (glGetError() != GL_NO_ERROR)
        return false;

    return true;
}

void cGraphicsSystemFSQ::terminate(void)
{
    if (m_VBO)
    {
        glDeleteBuffers(1, &m_VBO);
        m_VBO = 0;
    }
    if (m_EBO)
    {
        glDeleteBuffers(1, &m_EBO);
        m_EBO = 0;
    }
    if (m_VAO)
    {
        glDeleteVertexArrays(1, &m_VAO);
        m_VAO = 0;
    }
    m_numElements = 0;
}

void cGraphicsSystemFSQ::process(float _delta)
{
    // Assumes a shader program is already bound.
    if (m_VAO && m_numElements)
    {
        glBindVertexArray(m_VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_numElements), GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

}
