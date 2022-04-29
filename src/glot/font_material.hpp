#pragma once

#include <string>
#include <glm/glm.hpp>
#include "shader_utils.hpp"

class FontMaterial
{
  public:
    FontMaterial(const std::string &font_altas_filename);
    ~FontMaterial();

    void use(const glm::vec3 &colour, const glm::mat3 &transform) const;

  private:
    unsigned int m_font_atlas_tex;
    Program m_shader;
};
