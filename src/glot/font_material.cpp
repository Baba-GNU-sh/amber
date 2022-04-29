#include "font_material.hpp"
#include "resources.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <stb_image/stb_image.h>
#include <glm/gtc/type_ptr.hpp>

FontMaterial::FontMaterial(const std::string &font_atlas_filename)
{
    // Load the font atlas into a texture
    int width, height, nrChannels;
    const auto font_atlas_filepath = Resources::find_font(font_atlas_filename);
    unsigned char *tex_data =
        stbi_load(font_atlas_filepath.c_str(), &width, &height, &nrChannels, 0);
    if (!tex_data)
    {
        throw std::runtime_error("Unable to load font map: " + std::string(stbi_failure_reason()));
    }

    glGenTextures(1, &m_font_atlas_tex);
    glBindTexture(GL_TEXTURE_2D, m_font_atlas_tex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, tex_data);

    stbi_image_free(tex_data);

    std::vector<Shader> shaders{
        Shader(Resources::find_shader("sprite/vertex.glsl"), GL_VERTEX_SHADER),
        Shader(Resources::find_shader("sprite/fragment.glsl"), GL_FRAGMENT_SHADER)};
    m_shader = Program(shaders);
}

FontMaterial::~FontMaterial()
{
    glDeleteTextures(1, &m_font_atlas_tex);
}

void FontMaterial::use(const glm::vec3 &colour, const glm::mat3 &transform) const
{
    m_shader.use();

    int uniform_id = m_shader.uniform_location("view_matrix");
    glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(transform[0]));

    uniform_id = m_shader.uniform_location("tint_colour");
    glUniform3fv(uniform_id, 1, &colour[0]);

    glBindTexture(GL_TEXTURE_2D, m_font_atlas_tex);
}
