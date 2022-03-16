#pragma once

#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <iostream>
#include <stdexcept>

#include "shader_utils.hpp"

#include <string_view>
#include <vector>

class TextRenderer
{
  public:
	TextRenderer();
	~TextRenderer();

	void draw_text(std::string_view text, glm::mat3x3 view_matrix)
	{
		for (auto c : text) {
			draw(c, view_matrix);
			view_matrix = glm::translate(view_matrix, glm::vec2(1, 0));
		}
	}

	void draw(char c, glm::mat3x3 view_matrix)
	{
		m_program.use();
		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

		view_matrix = glm::translate(view_matrix, glm::vec2(0, 0));

		glUniformMatrix3fv(
		  m_uniform_view_matrix, 1, GL_FALSE, glm::value_ptr(view_matrix[0]));

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
		const int CHARWD = 4;

		int tl = x * CHARSZ;
		int y_px = y * CHARSZ;

		m_charbox_verts[2] = static_cast<float>(x * CHARSZ) / WIDTH;
		m_charbox_verts[3] = static_cast<float>(y * CHARSZ) / HEIGHT;
		m_charbox_verts[6] = static_cast<float>(x * CHARSZ + CHARWD) / WIDTH;
		m_charbox_verts[7] = static_cast<float>(y * CHARSZ) / HEIGHT;
		m_charbox_verts[10] = static_cast<float>(x * CHARSZ) / WIDTH;
		m_charbox_verts[11] = static_cast<float>(y * CHARSZ + CHARSZ) / HEIGHT;
		m_charbox_verts[14] = static_cast<float>(x * CHARSZ + CHARWD) / WIDTH;
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
	GLint m_uniform_view_matrix;
};
