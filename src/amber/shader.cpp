#include <glad/glad.h>
#include <sstream>
#include "shader_utils.hpp"

using namespace amber;

ShaderImpl::ShaderImpl(const std::string &filename, int shader_type) : m_filename(filename)
{
    std::ifstream ifs(m_filename, std::ios::in);
    if (!ifs.is_open())
    {
        throw std::runtime_error("Error loading shader: " + m_filename);
    }

    std::string content;
    std::string line = "";
    while (!ifs.eof())
    {
        std::getline(ifs, line);
        content.append(line + "\n");
    }

    m_shader_handle = glCreateShader(shader_type);

    const char *content_as_cstr = content.c_str();
    glShaderSource(m_shader_handle, 1, &content_as_cstr, NULL);

    glCompileShader(m_shader_handle);

    // check for shader compile errors
    int success;
    glGetShaderiv(m_shader_handle, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(m_shader_handle, 512, NULL, infoLog);

        std::stringstream error_msg;
        error_msg << "Shader compilation failed (" << m_filename << "): " << infoLog;

        glDeleteShader(m_shader_handle);

        throw std::runtime_error(error_msg.str());
    }
}

ShaderImpl::~ShaderImpl()
{
    glDeleteShader(m_shader_handle);
}

int ShaderImpl::get_handle() const
{
    return m_shader_handle;
}
