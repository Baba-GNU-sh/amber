#pragma once

#include <string>
#include <glm/glm.hpp>
#include "shader_utils.hpp"

namespace amber
{
class Font
{
  public:
    Font(const std::string &font_altas_filename);
    ~Font();
    Font(const Font &) = delete;
    Font(Font &&) = delete;
    Font &operator=(const Font &) = delete;
    Font &operator=(Font &&) = delete;

    void use(const glm::vec3 &colour, const glm::mat3 &transform) const;

  private:
    unsigned int m_font_atlas_tex;
    Program m_shader;
};
} // namespace amber
