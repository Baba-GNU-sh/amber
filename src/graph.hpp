#pragma once

#include <glad/glad.h> // Keep this one before glfw to avoid errors

#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>

#include "shader_utils.hpp"
#include "plot.hpp"

// #include "font.hpp"
#include <glm/glm.hpp>

struct GlyphVertex
{
	glm::vec2 vert;
	glm::vec2 tex_coords;
};

struct GlyphData
{
	GlyphVertex verts[4]; // Order: [TL, TR, BL, BR]
};

enum class LabelAlignment
{
	Left,
	Right,
	Center
};

enum class LabelAlignmentVertical
{
	Top,
	Center,
	Bottom
};

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
		const float zoom_delta = 1.0f + (yoffset / 10.0f);
		glm::vec2 zoom_delta_vec(1.0);

		// Is the cursor in the gutter?
		if (_hittest(_cursor, glm::vec2(0, 0), glm::vec2(GUTTER_SIZE_PX, _size.y - GUTTER_SIZE_PX)))
		{
			// We are in the y gutter
			zoom_delta_vec.y = zoom_delta;
		}
		else if (_hittest(_cursor, glm::vec2(GUTTER_SIZE_PX, _size.y - GUTTER_SIZE_PX), glm::vec2(_size.x, _size.y)))
		{
			zoom_delta_vec.x = zoom_delta;
		}
		else if (_hittest(_cursor, glm::vec2(GUTTER_SIZE_PX, 0), glm::vec2(_size.x, _size.y - GUTTER_SIZE_PX)))
		{
			zoom_delta_vec = glm::vec2(zoom_delta);
		}

		// Work out where the pointer is in graph space
		auto cursor_in_gs_old = screen2graph(_cursor);
		
		_update_view_matrix(glm::scale(_view_matrix, zoom_delta_vec));

		auto cursor_in_gs_new = screen2graph(_cursor);
		auto cursor_delta = cursor_in_gs_new - cursor_in_gs_old;

		_update_view_matrix(glm::translate(_view_matrix, cursor_delta));
	}

	/**
	 * @brief Update the matrix describing the transform from screen space to pixels.
	 *
	 * @param viewport_matrix The new value of the viewport matrix.
	 */
	void update_viewport_matrix(const glm::mat3x3& viewport_matrix)
	{
		_viewport_matrix = viewport_matrix;
		_viewport_matrix_inv = glm::inverse(viewport_matrix);
		_plot.update_viewport_matrix(viewport_matrix);
	}

	void draw() const;

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

	void set_plot_thickness(int thickness_px)
	{
		_plot.set_line_thickness(thickness_px);
	}

	int *get_plot_thickness()
	{
		return _plot.get_line_thickness();
	}

  private:

    bool _hittest(glm::vec2 value, glm::vec2 tl, glm::vec2 br)
	{
		if (value.x < tl.x) return false;
		if (value.x > br.x) return false;
		if (value.y < tl.y) return false;
		if (value.y > br.y) return false;
		return true;
	}

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
	void _draw_lines() const;
	void _draw_labels() const;
	void _draw_label(const std::string_view text, const glm::vec2 &pos, float height, float width, LabelAlignment align, LabelAlignmentVertical valign) const;
	void _draw_glyph(char c, const glm::vec2 &pos, float height, float width, GlyphData **buf) const;
	
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
	GLuint _glyphbuf_ebo;
	Program _glyph_shader;
	GLuint _glyph_texture;
	int _glyph_offset = 0;

	const int GUTTER_SIZE_PX = 60;
	const int TICKLEN = 10;
	const int SAMPLE_COUNT = 4096;

	Plot _plot;
};