#pragma once

#include <glad/glad.h> // Keep this one before glfw to avoid errors

#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>

#include "shader_utils.hpp"

// #include "font.hpp"
#include <glm/glm.hpp>

/**
 * @brief Stores and renders a graph with axes and zoom and pan mouse controls.
 */
class GraphView
{
  public:
	GraphView();
	~GraphView();

	/**
	 * @brief Update the size of the graph in the viewport.
	 *
	 * @param width The new width of the graph view in pixels.
	 * @param height The new height of the graph view in pixels.
	 */
	void set_size(int width, int height)
	{
		_size = glm::vec2(width, height);
	}

	/**
	 * @brief Get the size of the graph in the viewport.
	 *
	 * @return glm::vec2 2D vector containing the size of the graph in pixels.
	 */
	glm::ivec2 size() const
	{
		return _size;
	}

	/**
	 * @brief Update the position of the graph in the viewport.
	 *
	 * @param x The new x position in pixels.
	 * @param y The new y position in pixels.
	 */
	void set_position(int x, int y)
	{
		_position = glm::vec2(x, y);
	}

	/**
	 * @brief Get the posiiton of the graph in the viewport.
	 *
	 * @return glm::vec2 2D vector containing the posiiton of the graph in
	 * pixels.
	 */
	glm::ivec2 position() const
	{
		return _position;
	}

	/**
	 * @brief Call this to pass mouse button events through from GLFW to control
	 * the graph.
	 */
	void mouse_button(int button, int action, int mods)
	{
		(void)mods;

		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
			_dragging = true;
		else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
			_dragging = false;
	}

	/**
	 * @brief Call this to pass through cursor move events from GLFW to control
	 * the graph.
	 */
	void cursor_move(double xpos, double ypos)
	{
		glm::ivec2 new_cursor(xpos, ypos);

		if (_dragging) {
			// auto cursor_delta = new_cursor - _cursor;
			// std::cout << "GraphView: Dragging: " << cursor_delta.x << ", "
			//           << cursor_delta.y << "\n";

			// Work out the cursor delta in graph space
			auto cursor_gs_old = screen2graph(_cursor);
			auto cursor_gs_new = screen2graph(new_cursor);
			auto cursor_gs_delta = cursor_gs_new - cursor_gs_old;

			_update_view_matrix(glm::translate(_view_matrix, cursor_gs_delta));
		}

		_cursor = new_cursor;
	}

	/**
	 * @brief Call this to pass through mouse scroll events from GLFW to control
	 * the graph.
	 */
	void mouse_scroll(double xoffset, double yoffset)
	{
		// Work out where the pointer is in graph space
		auto cursor_in_gs_old = screen2graph(_cursor);
		float zoom_delta = 1.0 + (yoffset / 10);

		_update_view_matrix(
		  glm::scale(_view_matrix, glm::vec2(zoom_delta, zoom_delta)));

		auto cursor_in_gs_new = screen2graph(_cursor);
		auto cursor_delta = cursor_in_gs_new - cursor_in_gs_old;

		_update_view_matrix(glm::translate(_view_matrix, cursor_delta));
	}

	/**
	 * @brief Updat the viewport matrix - this should only happen when the view
	 * is resized.
	 *
	 * @param viewport_matrix The new value of the viewport matrix.
	 */
	void update_viewport_matrix(const glm::mat3x3& viewport_matrix)
	{
		_viewport_matrix = viewport_matrix;
		_viewport_matrix_inv = glm::inverse(viewport_matrix);
	}

	void draw() const
	{
		_draw_lines();
		_draw_labels();
		_draw_plot();
		return;

		// glBindVertexArray(m_vao);
		// glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

		// glm::mat3 ident(1.0);
		// glUniformMatrix3fv(
		//   m_uniform_view_matrix, 1, GL_FALSE, glm::value_ptr(ident[0]));

		// int margin_px = 60;
		// const int TICKLEN = 6;
		// const int TEXT_SPACING = 30;
		// int width = _size.x;
		// int height = _size.y;

		// // TODO workout what the tick spacing should be
		// glm::vec3 a(0.0f, 0.0f, 1.0f);
		// glm::vec3 b(1.0f, 1.0f, 1.0f);

		// auto a_ss = _view_matrix * a;
		// auto b_ss = _view_matrix * b;

		// auto delta = b_ss - a_ss;

		// const float ONSCREEN_TICKS = 0.2f;
		// float tick_spacing = (ONSCREEN_TICKS / delta.y);
		// tick_spacing = powf(2.0f, floorf(log2f(tick_spacing)));

		// {
		// 	// draw a line from margin_px to height - margin_px
		// 	glm::vec2 start_px(margin_px, margin_px);
		// 	glm::vec2 end_px(margin_px, height - margin_px);
		// 	auto start_clipspace =
		// 	  _viewport_matrix_inv * glm::vec3(start_px, 1.0);
		// 	;
		// 	auto end_clipspace = _viewport_matrix_inv * glm::vec3(end_px, 1.0);
		// 	draw_line_clipspace(start_clipspace, end_clipspace);

		// 	// Work out where (in graph space) margin and height-margin is
		// 	auto top_gs = _view_matrix_inv *
		// 	              (_viewport_matrix_inv * glm::vec3(0, margin_px, 1.0));
		// 	auto bottom_gs =
		// 	  _view_matrix_inv *
		// 	  (_viewport_matrix_inv * glm::vec3(0, height - margin_px, 1.0));

		// 	float start = ceilf(bottom_gs.y / tick_spacing) * tick_spacing;
		// 	float end = ceilf(top_gs.y / tick_spacing) * tick_spacing;

		// 	// Place a tick at every unit up the y axis
		// 	for (float i = start; i < end; i += tick_spacing) {
		// 		m_program.use();
		// 		glBindVertexArray(m_vao);
		// 		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

		// 		auto tick_y_vpspace =
		// 		  _viewport_matrix *
		// 		  (_view_matrix * glm::vec3(0.0f, static_cast<float>(i), 1.0f));
		// 		auto tick_start =
		// 		  _viewport_matrix_inv *
		// 		  glm::vec3(margin_px - TICKLEN, tick_y_vpspace.y, 1);
		// 		auto tick_end = _viewport_matrix_inv *
		// 		                glm::vec3(margin_px, tick_y_vpspace.y, 1);
		// 		draw_line_clipspace(tick_start, tick_end);

		// 		char buf[16];
		// 		std::snprintf(buf, 15, "%f", static_cast<float>(i));

		// 		// Glyphs should be 8*16 pixels
		// 		glm::vec2 text_position(margin_px - TICKLEN - TEXT_SPACING,
		// 		                        tick_y_vpspace.y);
		// 		glm::vec2 glyph_size(8.0f, 16.0f);

		// 		auto text_position_tl =
		// 		  _viewport_matrix_inv * glm::vec3(text_position, 1.0);
		// 		auto text_position_tr =
		// 		  _viewport_matrix_inv *
		// 		  glm::vec3((text_position + glyph_size), 1.0);

		// 		auto scale = text_position_tr - text_position_tl;

		// 		glm::mat3x3 m(1.0);
		// 		m = glm::translate(
		// 		  m, glm::vec2(text_position_tl.x, text_position_tl.y));
		// 		m = glm::scale(m, glm::vec2(scale.x, scale.y));
		// 		text.draw_text(buf, m);
		// 	}
		// }

		// {
		// 	m_program.use();
		// 	glBindVertexArray(m_vao);
		// 	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

		// 	// draw a line from margin_px to height - margin_px
		// 	glm::vec2 start_px(margin_px, height - margin_px);
		// 	glm::vec2 end_px(width - margin_px, height - margin_px);
		// 	auto start_clipspace =
		// 	  _viewport_matrix_inv * glm::vec3(start_px, 1.0);
		// 	;
		// 	auto end_clipspace = _viewport_matrix_inv * glm::vec3(end_px, 1.0);
		// 	draw_line_clipspace(start_clipspace, end_clipspace);

		// 	auto left_gs =
		// 	  _view_matrix_inv *
		// 	  (_viewport_matrix_inv * glm::vec3(margin_px, 0.0f, 1.0f));
		// 	auto right_gs =
		// 	  _view_matrix_inv *
		// 	  (_viewport_matrix_inv * glm::vec3(width - margin_px, 0.0f, 1.0f));

		// 	float start = ceilf(left_gs.x / tick_spacing) * tick_spacing;
		// 	float end = ceilf(right_gs.x / tick_spacing) * tick_spacing;

		// 	// Place a tick at every unit up the y axis
		// 	for (float i = start; i < end; i += tick_spacing) {
		// 		m_program.use();
		// 		glBindVertexArray(m_vao);
		// 		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

		// 		auto tick_x_vpspace =
		// 		  _viewport_matrix *
		// 		  (_view_matrix * glm::vec3(static_cast<float>(i), 0.0f, 1.0f));
		// 		auto tick_start = _viewport_matrix_inv *
		// 		                  glm::vec3(tick_x_vpspace.x,
		// 		                            height - (margin_px - TICKLEN),
		// 		                            1);
		// 		auto tick_end =
		// 		  _viewport_matrix_inv *
		// 		  glm::vec3(tick_x_vpspace.x, height - margin_px, 1);
		// 		draw_line_clipspace(tick_start, tick_end);

		// 		char buf[16];
		// 		std::snprintf(buf, 15, "%f", static_cast<float>(i));

		// 		// Glyphs should be 8*16 pixels
		// 		glm::vec2 text_position(tick_x_vpspace.x,
		// 		                        (height - margin_px) + TEXT_SPACING);
		// 		glm::vec2 glyph_size(8.0f, 16.0f);

		// 		auto text_position_tl =
		// 		  _viewport_matrix_inv * glm::vec3(text_position, 1.0);
		// 		auto text_position_tr =
		// 		  _viewport_matrix_inv *
		// 		  glm::vec3((text_position + glyph_size), 1.0);

		// 		auto scale = text_position_tr - text_position_tl;

		// 		glm::mat3x3 m(1.0);
		// 		m = glm::translate(
		// 		  m, glm::vec2(text_position_tl.x, text_position_tl.y));
		// 		m = glm::scale(m, glm::vec2(scale.x, scale.y));
		// 		text.draw_text(buf, m);
		// 	}
		// }

		// m_program.use();
		// glUniformMatrix3fv(
		//   m_uniform_view_matrix, 1, GL_FALSE, glm::value_ptr(_view_matrix[0]));

		// glBindVertexArray(m_plot_vao);
		// glDrawArrays(GL_LINE_STRIP, 0, 1000);
	}

	glm::mat3x3 get_view_matrix() const
	{
		return _view_matrix;
	}

	/**
	 * @brief Get the cursor's position in graph space.
	 *
	 * @return glm::vec2
	 */
	glm::vec2 get_cursor_graphspace() const
	{
		return screen2graph(_cursor);
	}

  private:
	/**
	 * @brief Converts a vector from viewport space (pixels w/ origin at TL) to
	 * graph space.
	 *
	 * @param value The vector to convert.
	 * @return glm::vec2 The resultant vector in graph space.
	 */
	glm::vec2 screen2graph(const glm::ivec2& value) const
	{
		glm::vec3 value3(value, 1.0f);
		auto value_cs = _viewport_matrix_inv * value3;
		auto value_gs = _view_matrix_inv * value_cs;
		return value_gs;
	}

	void _update_view_matrix(const glm::mat3x3& value)
	{
		_view_matrix = value;
		_view_matrix_inv = glm::inverse(value);
	}

	void _init_line_buffers();
	void _init_glyph_buffers();
	void _init_plot_buffers();
	void _draw_lines() const;
	void _draw_labels() const;
	void _draw_label(const std::string_view text, const glm::vec2 &pos, float size) const;
	void _draw_glyph(char c, const glm::vec2 &pos, float height) const;
	void _draw_plot() const;
	
	std::tuple<glm::vec2, glm::vec2, glm::ivec2> _get_tick_spacing() const;

	glm::ivec2 _position;
	glm::ivec2 _size;
	glm::vec2 _cursor;

	bool _dragging;

	glm::mat3x3 _view_matrix; // Transform from graph space to clip space
	glm::mat3x3 _view_matrix_inv;
	glm::mat3x3 _viewport_matrix; // Transform from clip space to screen space
	glm::mat3x3 _viewport_matrix_inv;

	// Line renderer
	GLuint _linebuf_vao;
	GLuint _linebuf_vbo;
	Program _lines_shader;

	// Glyph renderer
	GLuint _glyphbuf_vao;
	GLuint _glyphbuf_vbo;
	Program _glyph_shader;
	GLuint _glyph_texture;

	// Plot renderer
	GLuint _plot_vao;
	GLuint _plot_vbo;
	glm::vec2 _plot_data[1024];

	// Program m_program;
	// GLuint m_vao;
	// GLuint m_vbo;
	// glm::vec2 m_verticies[2];

	// GLuint m_plot_vao, m_plot_vbo;
	// std::array<glm::vec2, 1000> m_plot_verticies;
	// GLint m_uniform_view_matrix;

	const int GUTTER_SIZE_PX = 60;
	const int TICKLEN = 10;
};