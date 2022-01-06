#include "shader_utils.hpp"
#include <glad/glad.h>

ShaderProgram::ShaderProgram(std::vector<std::shared_ptr<Shader>> &&shaders)
{
    m_program_handle = glCreateProgram();

    for (const auto &shader : shaders)
    {
        glAttachShader(m_program_handle, shader->get_handle());
    }

    glLinkProgram(m_program_handle);

    // Check for linking errors
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

ShaderProgram::~ShaderProgram()
{
    glDeleteProgram(m_program_handle);
}

int ShaderProgram::get_handle()
{
    return m_program_handle;
}

int ShaderProgram::get_uniform_location(const char *uniform_name)
{
    return glGetUniformLocation(m_program_handle, uniform_name);
}
