#include "graph.hpp"
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iomanip>
#include <sstream>
#include <iostream>

GraphView::GraphView()
  : _position(0, 0)
  , _size(100, 100) // This is completely arbitrary
  , _dragging(false)
{
	update_viewport_matrix(glm::mat3x3(1.0f));
	_update_view_matrix(glm::mat3x3(1.0f));

	// The idea is that we need to draw some lines, and some text.
	// The axes are just long lines, with smaller lines indicating the
	// ticks. There are some text labels on each of the ticks, and labels on
	// the axes indicating the units / scaling etc. Text is rendered using
	// bitmap fonts, and is made up of glyphs rendered using one quad per
	// glyph. When render is called, the verticies are updated, transferred
	// to the GPU, then two draw calls are made, one to render the lines,
	// and one to render the glyphs.

	_init_line_buffers();
	_init_glyph_buffers();
	_init_plot_buffers();
}

GraphView::~GraphView()
{

}

void GraphView::_init_line_buffers()
{
	glGenVertexArrays(1, &_linebuf_vao);
	glBindVertexArray(_linebuf_vao);

	glGenBuffers(1, &_linebuf_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _linebuf_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 1024, nullptr, GL_STREAM_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
	glEnableVertexAttribArray(0);

	std::vector<Shader> shaders{ Shader("simple_vertex.glsl", GL_VERTEX_SHADER),
		                         Shader("simple_fragment.glsl",
		                                GL_FRAGMENT_SHADER) };
	_lines_shader = Program(shaders);
}

void GraphView::_init_glyph_buffers()
{
	glGenVertexArrays(1, &_glyphbuf_vao);
	glBindVertexArray(_glyphbuf_vao);

	glGenBuffers(1, &_glyphbuf_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _glyphbuf_vbo);

	glGenBuffers(1, &_glyphbuf_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _glyphbuf_ebo);

	const int sz = 6 * 128 / 4;
	unsigned int indices[sz];
	for (int i = 0, j=0; j < sz; i+=4, j+=6)
	{
		indices[j] = i;
		indices[j+1] = i+1;
		indices[j+2] = i+2;
		indices[j+3] = i+1;
		indices[j+4] = i+2;
		indices[j+5] = i+3;
	}

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); 

	// A glyph is rendered as a quad so we only need 4 verts and 4 texture lookups
	glBufferData(GL_ARRAY_BUFFER, 128 * sizeof(GlyphData), nullptr, GL_STREAM_DRAW);

	// Define an attribute for the glyph verticies
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), (void*)0);
	glEnableVertexAttribArray(0);

	// Define an attribute for the texture lookups
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GlyphVertex), (void*)offsetof(GlyphVertex, tex_coords));
	glEnableVertexAttribArray(1);

	std::vector<Shader> shaders{ Shader("char_vert.glsl", GL_VERTEX_SHADER),
		                         Shader("char_frag.glsl", GL_FRAGMENT_SHADER) };
	_glyph_shader = Program(shaders);

	int width, height, nrChannels;
	unsigned char *tex_data = stbi_load("font.png", &width, &height, &nrChannels, 0);
	if (!tex_data) {
		throw std::runtime_error("Unable to load font map: " + std::string(stbi_failure_reason()));
	}

	glGenTextures(1, &_glyph_texture);
	glBindTexture(GL_TEXTURE_2D, _glyph_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// glGenerateMipmap(GL_TEXTURE_2D);

	glTexImage2D(GL_TEXTURE_2D,
	             0,
	             GL_RED,
	             width,
	             height,
	             0,
	             GL_RED,
	             GL_UNSIGNED_BYTE,
	             tex_data);
	
	stbi_image_free(tex_data);
}

void GraphView::_init_plot_buffers()
{
	glGenVertexArrays(1, &_plot_vao);
	glBindVertexArray(_plot_vao);

	glGenBuffers(1, &_plot_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _plot_vbo);
	glBufferData(GL_ARRAY_BUFFER,  sizeof(float) * 4 * SAMPLE_COUNT, nullptr, GL_STREAM_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
	glEnableVertexAttribArray(0);
}

void GraphView::draw() const
{
	_draw_lines();
	_draw_labels();
	_draw_plot();
}

void GraphView::_draw_lines() const
{
	int offset = 0;

	auto tick_spacing = _get_tick_spacing();
	auto tick_spacing_major = std::get<0>(tick_spacing);
	auto tick_spacing_minor = std::get<1>(tick_spacing);

	glBindVertexArray(_linebuf_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _linebuf_vbo);

	// Get a pointer to the underlying buffer
	void *raw_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	auto *ptr = reinterpret_cast<glm::vec2 *>(raw_ptr);

	glm::vec2 tl(GUTTER_SIZE_PX, 0);
	glm::vec2 bl(GUTTER_SIZE_PX, _size.y - GUTTER_SIZE_PX);
	glm::vec2 br(_size.x, _size.y - GUTTER_SIZE_PX);

	// Draw the y axis line
	ptr[offset++] = tl;
	ptr[offset++] = bl;

	// Draw the x axis line
	ptr[offset++] = bl;
	ptr[offset++] = br;

	auto draw_ticks = [&](const glm::vec2 &tick_spacing, const glm::vec2 &tick_size_y, const glm::vec2 &tick_size_x)
	{
		// Draw the y-axis ticks
		// Work out where (in graph space) margin and height-margin is
		auto top_gs = screen2graph(tl);
		auto bottom_gs = screen2graph(bl);
		float start = ceilf(bottom_gs.y / tick_spacing.y) * tick_spacing.y;
		float end = ceilf(top_gs.y / tick_spacing.y) * tick_spacing.y;

		// Place a tick at every unit up the y axis
		for (float i = start; i < end; i += tick_spacing.y) {
			auto tick_y_vpspace = _viewport_matrix * (_view_matrix * glm::vec3(0.0f, i, 1.0f));
			ptr[offset++] = glm::vec2(GUTTER_SIZE_PX, tick_y_vpspace.y) + tick_size_y;
			ptr[offset++] = glm::vec2(GUTTER_SIZE_PX, tick_y_vpspace.y);
		}

		// Draw the y axis ticks
		auto left_gs = screen2graph(bl);
		auto right_gs = screen2graph(br);
		start = ceilf(left_gs.x / tick_spacing.x) * tick_spacing.x;
		end = ceilf(right_gs.x / tick_spacing.x) * tick_spacing.x;

		// Place a tick at every unit along the x axis
		for (float i = start; i < end; i += tick_spacing.x) {
			auto tick_x_vpspace = _viewport_matrix * (_view_matrix * glm::vec3(i, 0.0f, 1.0f));
			ptr[offset++] = glm::vec2(tick_x_vpspace.x, _size.y - GUTTER_SIZE_PX) + tick_size_x;
			ptr[offset++] = glm::vec2(tick_x_vpspace.x, _size.y - GUTTER_SIZE_PX);
		}
	};

	draw_ticks(tick_spacing_major, glm::vec2(-TICKLEN, 0), glm::vec2(0, TICKLEN));
	draw_ticks(tick_spacing_minor, glm::vec2(-TICKLEN/2, 0), glm::vec2(0, TICKLEN/2));

	// make sure to tell OpenGL we're done with the pointer
	glUnmapBuffer(GL_ARRAY_BUFFER);

	_lines_shader.use();
	int uniform_id = _lines_shader.get_uniform_location("view_matrix");
	glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(_viewport_matrix_inv[0]));
	glDrawArrays(GL_LINES, 0, offset);
}

void GraphView::_draw_labels() const
{
	glm::vec2 tl(GUTTER_SIZE_PX, 0);
	glm::vec2 bl(GUTTER_SIZE_PX, _size.y - GUTTER_SIZE_PX);
	glm::vec2 br(_size.x, _size.y - GUTTER_SIZE_PX);
	auto tick_spacing = _get_tick_spacing();
	auto tick_spacing_major = std::get<0>(tick_spacing);
	auto tick_spacing_minor = std::get<1>(tick_spacing);
	auto precision = std::get<2>(tick_spacing);

	// Draw one label per tick on the y axis
	auto top_gs = screen2graph(tl);
	auto bottom_gs = screen2graph(bl);
	float start = ceilf(bottom_gs.y / tick_spacing_major.y) * tick_spacing_major.y;
	float end = ceilf(top_gs.y / tick_spacing_major.y) * tick_spacing_major.y;

	// Place a tick at every unit up the y axis
	for (float i = start; i < end; i += tick_spacing_major.y) {
		auto tick_y_vpspace = _viewport_matrix * (_view_matrix * glm::vec3(0.0f, i, 1.0f));
		glm::vec2 point(GUTTER_SIZE_PX - TICKLEN, tick_y_vpspace.y);

		std::stringstream ss;
		ss << std::fixed << std::setprecision(precision.y) << i;
		_draw_label(ss.str(), point, 16, LabelAlignment::Right, LabelAlignmentVertical::Center);
	}

	// Draw the y axis ticks
	auto left_gs = screen2graph(bl);
	auto right_gs = screen2graph(br);
	start = ceilf(left_gs.x / tick_spacing_major.x) * tick_spacing_major.x;
	end = ceilf(right_gs.x / tick_spacing_major.x) * tick_spacing_major.x;

	// Place a tick at every unit along the x axis
	for (float i = start; i < end; i += tick_spacing_major.x) {
		auto tick_x_vpspace = _viewport_matrix * (_view_matrix * glm::vec3(i, 0.0f, 1.0f));
		glm::vec2 point(tick_x_vpspace.x, _size.y - GUTTER_SIZE_PX + TICKLEN);

		std::stringstream ss;
		ss << std::fixed << std::setprecision(precision.x) << i;
		_draw_label(ss.str(), point, 16, LabelAlignment::Center, LabelAlignmentVertical::Top);
	}
}

void GraphView::_draw_label(const std::string_view text, const glm::vec2 &pos, float size, LabelAlignment align, LabelAlignmentVertical valign) const
{
	glm::vec2 offset = pos;
	glm::vec2 delta = glm::vec2(size/2, 0);

	if (align == LabelAlignment::Right)
	{
		offset -= delta * static_cast<float>(text.size());
	}
	else if (align == LabelAlignment::Center)
	{
		offset -= delta * static_cast<float>(text.size()) / 2.0f;
	}

	if (valign == LabelAlignmentVertical::Center)
	{
		offset -= glm::vec2(0, size/2);
	}
	else if (valign == LabelAlignmentVertical::Bottom)
	{
		offset -= glm::vec2(0, size);
	}

	GlyphData buffer[128];
	GlyphData *bufptr = &buffer[0];

	for (auto c : text)
	{
		_draw_glyph(c, offset, size, &bufptr);
		offset += delta;
	}

	std::size_t count = (bufptr - buffer);

	glBindVertexArray(_glyphbuf_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _glyphbuf_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GlyphData) * count, buffer);

	_glyph_shader.use();
	int uniform_id = _glyph_shader.get_uniform_location("view_matrix");
	glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(_viewport_matrix_inv[0]));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _glyphbuf_ebo);
	glDrawElements(GL_TRIANGLES, 6 * count, GL_UNSIGNED_INT, 0);
	// glDrawArrays(GL_TRIANGLES, 0, 4 * (bufptr - buffer));
}

void GraphView::_draw_glyph(char c, const glm::vec2 &pos, float size, GlyphData **buf) const
{
	const float WIDTH = size / 2;
	GlyphData *data = *buf;
	data->verts[0].vert = pos;
	data->verts[1].vert = pos + glm::vec2(WIDTH, 0.0f);
	data->verts[2].vert = pos + glm::vec2(0.0f, size);
	data->verts[3].vert = pos + glm::vec2(WIDTH, size);

	// Look up the texture coordinate for the character
	const int COLS = 16;
	const int ROWS = 8;
	int col = c % COLS;
	int row = c / COLS;
	const float COL_STRIDE = 1.0f / COLS;
	const float GLYPH_WIDTH = 0.5f / COLS;
	const float ROW_STRIDE = 1.0f / ROWS;
	const float GLYPH_HEIGHT = 1.0f / ROWS;

	data->verts[0].tex_coords = glm::vec2(COL_STRIDE * col, ROW_STRIDE * row);
	data->verts[1].tex_coords = glm::vec2(COL_STRIDE * col + GLYPH_WIDTH, ROW_STRIDE * row);
	data->verts[2].tex_coords = glm::vec2(COL_STRIDE * col, ROW_STRIDE * row + GLYPH_HEIGHT);
	data->verts[3].tex_coords = glm::vec2(COL_STRIDE * col + GLYPH_WIDTH, ROW_STRIDE * row + GLYPH_HEIGHT);

	*buf = data + 1;
}

void GraphView::_draw_plot() const
{
	glBindVertexArray(_plot_vao);
	glBindBuffer(GL_ARRAY_BUFFER, _plot_vbo);

	_lines_shader.use();
	int uniform_id = _lines_shader.get_uniform_location("view_matrix");
	glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(_view_matrix[0]));
	
	auto time = glfwGetTime();
	glm::vec2 plot_data[SAMPLE_COUNT];
	for (int i = 0; i < SAMPLE_COUNT; i++)
	{
		const float x = static_cast<float>(i-SAMPLE_COUNT/2) / 400.0f;
		plot_data[i].x = x;
		plot_data[i].y = sinf(100.0f * x + time) * sinf(1.0f * x);
	}

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(plot_data), &plot_data);
	glDrawArrays(GL_LINE_STRIP, 0, SAMPLE_COUNT);
}

std::tuple<glm::vec2, glm::vec2, glm::ivec2> GraphView::_get_tick_spacing() const
{
	const glm::vec2 MIN_TICK_SPACING_PX(80, 50);

	// Calc the size of this vector in graph space (ignoring translation & sign)
	const glm::vec2 min_tick_spacing_gs = glm::abs(
		screen2graph(glm::vec2(0.0f, 0.0f)) 
		- screen2graph(MIN_TICK_SPACING_PX));

	// Round this size up to the nearest power of 10
	glm::vec2 tick_spacing;
	tick_spacing.x = powf(10.0f, ceilf(log10f(min_tick_spacing_gs.x)));
	tick_spacing.y = powf(10.0f, ceilf(log10f(min_tick_spacing_gs.y)));
	glm::vec2 minor_tick_spacing = tick_spacing / 2.0f;

	glm::ivec2 precision;
	precision.x = -ceilf(log10f(min_tick_spacing_gs.x));
	precision.y = -ceilf(log10f(min_tick_spacing_gs.y));

	auto scale = min_tick_spacing_gs / tick_spacing;
	if (scale.x < 0.5f)
	{
		tick_spacing.x /= 2;
		++precision.x;
		minor_tick_spacing.x = tick_spacing.x / 5;
	}

	if (scale.y < 0.5f)
	{
		tick_spacing.y /= 2;
		++precision.y;
		minor_tick_spacing.y = tick_spacing.y / 5;
	}

	if (precision.x < 0) precision.x = 0;
	if (precision.y < 0) precision.y = 0;

	return std::tuple(tick_spacing, minor_tick_spacing, precision);
}
