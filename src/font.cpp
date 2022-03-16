#include "font.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

TextRenderer::TextRenderer()
  : m_charbox_verts{ 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
	                 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f }
{
	int width, height, nrChannels;
	m_tex_data = stbi_load("font.tga", &width, &height, &nrChannels, 0);
	if (!m_tex_data) {
		throw std::runtime_error("Unable to load font");
	}

	std::cout << "NChannels: " << nrChannels << '\n';

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
	             GL_RED,
	             width,
	             height,
	             0,
	             GL_RED,
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

	glVertexAttribPointer(
	  1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

	std::vector<Shader> shaders{ Shader("char_vert.glsl", GL_VERTEX_SHADER),
		                         Shader("char_frag.glsl", GL_FRAGMENT_SHADER) };

	m_program = Program(shaders);
	m_uniform_view_matrix = m_program.get_uniform_location("view_matrix");
}

TextRenderer::~TextRenderer()
{
	stbi_image_free(m_tex_data);
}
