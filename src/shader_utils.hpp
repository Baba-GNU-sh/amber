#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>

class ShaderImpl
{
public:
    ShaderImpl(const std::string &filename, int shader_type);
    ~ShaderImpl();

    ShaderImpl(const ShaderImpl &other) = delete;
    ShaderImpl &operator=(const ShaderImpl &other) = delete;

    int get_handle() const;

private:
    const std::string m_filename;
    int m_shader_handle;
};

/**
 * @brief Little wrapper for managing a shader loaded from disk.
 * Manages the actual loading from a file and handles errors.
 */
class Shader
{
public:
    /**
     * @brief Initialize the shader from a file.
     * 
     * @param filename The filename where the shader program is stored.
     * @param shader_type The type of the shader to create (vertex, geometry, fragment), passed straight into glCreateShader.
     */
    Shader(const std::string &filename, int shader_type)
    {
        m_impl = std::make_shared<ShaderImpl>(filename, shader_type);
    }

    /**
     * @brief Get the OpenGL handle for this shader.
     */
    int get_handle() const
    {
        return m_impl->get_handle();
    }

private:
    std::shared_ptr<ShaderImpl> m_impl;
};

class ProgramImpl
{
public:
    ProgramImpl(const std::vector<Shader> &);
    ~ProgramImpl();

    ProgramImpl(const ProgramImpl &other) = delete;
    ProgramImpl &operator=(const ProgramImpl &other) = delete;

    void use() const;
    int get_handle() const;
    int get_uniform_location(const char *uniform_name) const;

private:
    int m_program_handle;
};

/**
 * @brief Little wrapper around a GL program, created and linked from a list of shaders.
 * Manages linking, and error handling.
 */
class Program
{
public:
    /**
     * @brief Creates a new shader program, getting
     * 
     * @param shaders 
     */
    Program(const std::vector<Shader> &shaders)
    {
        m_impl = std::make_shared<ProgramImpl>(shaders);
    }

    Program() = default;

    /**
     * @brief Get the OpenGL program handle.
     */
    int get_handle() const
    {
        return m_impl->get_handle();
    }

    /**
     * @brief Select this program for use.
     */
    void use() const
    {
        m_impl->use();
    }

    /**
     * @brief Lookup a uniform location.
     * 
     * @param uniform_name The name of the uniform to lookup.
     * @return int The value of the uniform's location in the program.
     */
    int get_uniform_location(const char *uniform_name) const
    {
        return m_impl->get_uniform_location(uniform_name);
    }


private:
    std::shared_ptr<ProgramImpl> m_impl;
};
