#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "shader_utils.hpp"
#include <vector>

class TextRenderer
{
  public:
	TextRenderer()
	  : m_charbox_verts{ 0.5f,  0.5f,  1.0f, 1.0f, 0.5f,  -0.5f, 1.0f, 0.0f,
		                 -0.5f, -0.5f, 0.0f, 0.0f, -0.5f, 0.5f,  0.0f, 1.0f }
	{
		int width, height, nrChannels;
		m_tex_data = stbi_load("font.png", &width, &height, &nrChannels, 0);
		if (!m_tex_data) {
			throw std::runtime_error("Unable to load font");
		}

		glGenVertexArrays(1, &m_vao);
		glBindVertexArray(m_vao);

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexImage2D(GL_TEXTURE_2D,
		             0,
		             GL_RGB,
		             width,
		             height,
		             0,
		             GL_RGB,
		             GL_UNSIGNED_BYTE,
		             m_tex_data);

		glGenBuffers(1, &m_vbo);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

		glBufferData(GL_ARRAY_BUFFER,
		             sizeof(m_charbox_verts),
		             m_charbox_verts,
		             GL_STATIC_DRAW);

		glVertexAttribPointer(
		  0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), m_charbox_verts);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1,
		                      2,
		                      GL_FLOAT,
		                      GL_FALSE,
		                      4 * sizeof(float),
		                      (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		std::vector<Shader> shaders{
            Shader("char_vert.glsl", GL_VERTEX_SHADER),
            Shader("char_frag.glsl", GL_FRAGMENT_SHADER)};

        m_program = Program(shaders);
	}

	~TextRenderer()
	{
		stbi_image_free(m_tex_data);
	}

	void draw()
	{
		m_program.use();
		glBindVertexArray(m_vao);
		glDrawArrays(GL_QUADS, 0, 4);
	}

  private:
	unsigned char* m_tex_data;
	unsigned int texture;
	float m_charbox_verts[16];
	GLuint m_vao, m_vbo;
	Program m_program;
};
