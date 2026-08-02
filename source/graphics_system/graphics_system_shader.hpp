
#ifndef GRAPHICS_SYSTEM_SHADER_HPP
#define GRAPHICS_SYSTEM_SHADER_HPP

#include <cstdint>
#include <string>

#include "galogen.hpp"
#include "../utils/file_utils.hpp"

class cGraphicsSystemShader
{
    public:
        cGraphicsSystemShader();
        ~cGraphicsSystemShader();

        // Interface
        std::uint32_t initialize(const std::string &_fileName);
        void          terminate(void);
        void          use(void);
        std::uint32_t getID(void) { return m_programID; };

        // Utility uniform functions
        std::uint32_t getUniformLocation(const std::string &_string);

    protected:

    private:
        std::uint32_t m_programID = 0;
};

#endif //GRAPHICS_SYSTEM_SHADER_HPP

