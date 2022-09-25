#include <glad/glad.h>
#include <sstream>
#include "shader_utils.hpp"

using namespace amber;

ProgramImpl::ProgramImpl(const std::vector<Shader> &shaders)
{
    m_program_handle = glCreateProgram();

    for (const auto &shader : shaders)
    {
        glAttachShader(m_program_handle, shader.get_handle());
    }

    glLinkProgram(m_program_handle);

    // Check for link errors
    int success;
    glGetProgramiv(m_program_handle, GL_LINK_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(m_program_handle, 512, NULL, infoLog);

        std::stringstream error_msg;
        error_msg << "Shader program link failed: " << infoLog;

        glDeleteProgram(m_program_handle);

        throw std::runtime_error(error_msg.str());
    }
}

ProgramImpl::~ProgramImpl()
{
    glDeleteProgram(m_program_handle);
}

int ProgramImpl::get_handle() const
{
    return m_program_handle;
}

int ProgramImpl::uniform_location(const char *uniform_name) const
{
    return glGetUniformLocation(m_program_handle, uniform_name);
}

void ProgramImpl::use() const
{
    glUseProgram(m_program_handle);
}
