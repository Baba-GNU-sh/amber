#pragma once

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "shader_utils.hpp"
#include "stb_image.h"
#include <vector>

class TextRenderer
{
  public:
	TextRenderer()
	  : m_charbox_verts{ 0.5f,  0.5f, 1.0f, 0.0f, 0.5f,  -0.5f, 1.0f, 1.0f,
		                 -0.5f, 0.5f, 0.0f, 0.0f, -0.5f, -0.5f, 0.0f, 1.0f }
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

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		// glGenerateMipmap(GL_TEXTURE_2D);

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

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1,
		                      2,
		                      GL_FLOAT,
		                      GL_FALSE,
		                      4 * sizeof(float),
		                      (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);

		std::vector<Shader> shaders{ Shader("char_vert.glsl", GL_VERTEX_SHADER),
			                         Shader("char_frag.glsl",
			                                GL_FRAGMENT_SHADER) };

		m_program = Program(shaders);
	}

	~TextRenderer()
	{
		stbi_image_free(m_tex_data);
	}

	void draw(char c)
	{
		m_program.use();
		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

		int col = (c) % 16;
		int row = (c) / 16;
		// glDrawArrays(GL_LINE_STRIP, 0, 4);
		select_char(col, row);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}

	void select_char(int x, int y)
	{
		const int WIDTH = 128;
		const int HEIGHT = 64;
		const int CHARSZ = 8;

		int tl = x * CHARSZ;
		int y_px = y * CHARSZ;

		m_charbox_verts[2] = static_cast<float>(x * CHARSZ + CHARSZ) / WIDTH;
		m_charbox_verts[3] = static_cast<float>(y * CHARSZ) / HEIGHT;
		m_charbox_verts[6] = static_cast<float>(x * CHARSZ + CHARSZ) / WIDTH;
		m_charbox_verts[7] = static_cast<float>(y * CHARSZ + CHARSZ) / HEIGHT;
		m_charbox_verts[10] = static_cast<float>(x * CHARSZ) / WIDTH;
		m_charbox_verts[11] = static_cast<float>(y * CHARSZ) / HEIGHT;
		m_charbox_verts[14] = static_cast<float>(x * CHARSZ) / WIDTH;
		m_charbox_verts[15] = static_cast<float>(y * CHARSZ + CHARSZ) / HEIGHT;

		glBufferSubData(
		  GL_ARRAY_BUFFER, 0, sizeof(m_charbox_verts), m_charbox_verts);
	}

  private:
	unsigned char* m_tex_data;
	unsigned int texture;
	float m_charbox_verts[16];
	GLuint m_vao, m_vbo;
	Program m_program;
};
