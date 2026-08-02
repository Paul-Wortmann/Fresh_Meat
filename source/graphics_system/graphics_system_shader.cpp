

#include "graphics_system_shader.hpp"

cGraphicsSystemShader::cGraphicsSystemShader()
{
    m_programID = 0;
}

cGraphicsSystemShader::~cGraphicsSystemShader()
{
    terminate();
}

std::uint32_t cGraphicsSystemShader::initialize(const std::string &_fileName)
{
    // Vertex shader
    std::string vertexSource = gFileToString(_fileName + ".vs");
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const GLchar *source = (const GLchar *)vertexSource.c_str();
    glShaderSource(vertexShader, 1, &source, 0);
    glCompileShader(vertexShader);
    GLint isCompiled = 0;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint infoLogLength = 0;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);
        char* infoLog = new char[infoLogLength];
        glGetShaderInfoLog(vertexShader, infoLogLength, &infoLogLength, &infoLog[0]);
        std::cout << "Problem with shader: " << std::string(_fileName + ".vs") << std::endl;
        std::cout << std::string(infoLog) << std::endl;
        glDeleteShader(vertexShader);
        delete[] infoLog;
        return EXIT_FAILURE;
    }

    // Fragment shader
    std::string fragmentSource = gFileToString(_fileName + ".fs");
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    source = (const GLchar *)fragmentSource.c_str();
    glShaderSource(fragmentShader, 1, &source, 0);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
        GLint infoLogLength = 0;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLogLength);
        char* infoLog = new char[infoLogLength];
        glGetShaderInfoLog(fragmentShader, infoLogLength, &infoLogLength, &infoLog[0]);
        std::cout << "Problem with shader: " << std::string(_fileName + ".fs") << std::endl;
        std::cout << std::string(infoLog) << std::endl;
        glDeleteShader(fragmentShader);
        glDeleteShader(vertexShader);
        delete[] infoLog;
        return EXIT_FAILURE;
    }

    // Shader Program
    m_programID = glCreateProgram();
    glAttachShader(m_programID, vertexShader);
    glAttachShader(m_programID, fragmentShader);
    glLinkProgram(m_programID);
    GLint isLinked = 0;
    glGetProgramiv(m_programID, GL_LINK_STATUS, (int *)&isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint infoLogLength = 0;
        glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &infoLogLength);
        char* infoLog = new char[infoLogLength];
        glGetProgramInfoLog(m_programID, infoLogLength, &infoLogLength, &infoLog[0]);
        std::cout << "Problem with shader program: " << std::string(_fileName) << std::endl;
        std::cout << std::string(infoLog) << std::endl;
        glDeleteProgram(m_programID);
        glDeleteShader(fragmentShader);
        glDeleteShader(vertexShader);
        delete[] infoLog;
        return EXIT_FAILURE;
    }

    // Detach shaders
    glDetachShader(m_programID, vertexShader);
    glDetachShader(m_programID, fragmentShader);

    // Cleanup shader objects (they are no longer needed)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return EXIT_SUCCESS;
}

void cGraphicsSystemShader::terminate(void)
{
    if (m_programID != 0)
    {
        glDeleteProgram(m_programID);
        m_programID = 0;
    }
}

std::uint32_t cGraphicsSystemShader::getUniformLocation(const std::string &_string)
{
    return (m_programID != 0) ? glGetUniformLocation(m_programID, _string.c_str()) : 0;
}

void cGraphicsSystemShader::use(void)
{
    if (m_programID != 0)
        glUseProgram(m_programID);
}
