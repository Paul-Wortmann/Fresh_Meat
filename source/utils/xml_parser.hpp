
#ifndef XML_PARSER_HPP
#define XML_PARSER_HPP

#include <cstdint>
#include <fstream>
#include <string>
#include <glm/glm.hpp>

// A class to hold an xml file content, and provide helper functions to extract data there from.
// *This does not cover the full XML specification, but a basic subset needed for this project.

class cXML
{
    public:
        std::uint32_t load(const std::string &_fileName);
        void          free(void) { delete [] m_line; m_line = nullptr; m_lineCount = 0; m_isValid = false;}
        bool          isValid(void) { return m_isValid; };
        std::uint32_t lineCount(void) { return m_lineCount; }
        std::string   line(std::uint32_t _lineNum) { return (_lineNum < m_lineCount) ? m_line[_lineNum] : ""; }
        std::uint32_t getLine(const std::string &_key, const std::uint32_t  _instance = 1);
        std::uint32_t getInstanceAfterLine(const std::string &_key, const std::uint32_t  _line);
        std::uint32_t getInstanceCount(const std::string &_key);
        std::string   getString(const std::string &_key, const std::uint32_t  _instance = 1);
        std::string   getValueFromString(const std::string &_string, const std::string &_keyID);
        std::string   getStringKeyValue(const std::string &_key, const std::string &_keyID, const std::uint32_t  _instance = 1);
        std::int32_t  getInteger(const std::string &_key, const std::uint32_t  _instance = 1);
        std::uint64_t getInteger64(const std::string &_key, const std::uint32_t  _instance = 1);
        float         getFloat(const std::string &_key, const std::uint32_t  _instance = 1);
        glm::vec4     getVec4(const std::string &_key, const std::uint32_t  _instance = 1);
        glm::vec3     getVec3(const std::string &_key, const std::uint32_t  _instance = 1);
        glm::vec2     getVec2(const std::string &_key, const std::uint32_t  _instance = 1);
        glm::ivec4    getIvec4(const std::string &_key, const std::uint32_t  _instance = 1);
        glm::ivec3    getIvec3(const std::string &_key, const std::uint32_t  _instance = 1);
        glm::ivec2    getIVec2(const std::string &_key, const std::uint32_t  _instance = 1);
        glm::mat4     getMat4(const std::string &_key, const std::uint32_t  _instance = 1);

    protected:

    private:
        std::string   m_formatLine(const std::string &_string);
        bool          m_isValid   = false;
        std::uint32_t m_lineCount = 0;
        std::string*  m_line      = nullptr;
};

#endif // XML_PARSER_HPP
