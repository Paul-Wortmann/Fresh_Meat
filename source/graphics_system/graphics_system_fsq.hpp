
// full screen quad

#ifndef GRAPHICS_SYSTEM_FSQ_HPP
#define GRAPHICS_SYSTEM_FSQ_HPP

#include <cstdint>
#include <vector>
#include <glm/glm.hpp>

#include "galogen.hpp"

class cGraphicsSystemFSQ
{
    public:
        // base interface
        bool initialize(void);
        void terminate(void);
        void process(float _delta);

    private:
        glm::mat4     m_modelMatrix = glm::mat4(1);
        std::uint32_t m_VBO = 0;
        std::uint32_t m_EBO = 0;
        std::uint32_t m_VAO = 0;
        std::uint32_t m_numElements = 0;

};

#endif // GRAPHICS_SYSTEM_FSQ_HPP
