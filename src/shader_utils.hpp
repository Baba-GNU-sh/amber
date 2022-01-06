#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>

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
    Shader(const std::string &filename, int shader_type);
    ~Shader();

    /**
     * @brief Get the OpenGL handle for this shader.
     */
    int get_handle();

private:
    const std::string m_filename;
    int m_shader_handle;
};

/**
 * @brief Little wrapper around a shader program, created and linked from a list of shaders.
 * Manages linking, and error handling.
 */
class ShaderProgram
{
public:
    /**
     * @brief Creates a new shader program, getting
     * 
     * @param shaders 
     */
    ShaderProgram(std::vector<std::shared_ptr<Shader>> &&shaders);
    ~ShaderProgram();

    /**
     * @brief Get the OpenGL program handle.
     */
    int get_handle();

    /**
     * @brief Lookup a uniform location.
     * 
     * @param uniform_name The name of the uniform to lookup.
     * @return int The value of the uniform's location in the program.
     */
    int get_uniform_location(const char *uniform_name);

private:
    int m_program_handle;
};
