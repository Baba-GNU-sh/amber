#include "graph.hpp"

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

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

	glBufferData(
	  GL_ARRAY_BUFFER, sizeof(m_verticies), m_verticies, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(
	  0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
	glEnableVertexAttribArray(0);

	// Generate buffers for the actual plot
	glGenVertexArrays(1, &m_plot_vao);
	glBindVertexArray(m_plot_vao);

	glGenBuffers(1, &m_plot_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, m_plot_vbo);

	for (int i = 0; i < m_plot_verticies.size(); i++) {
		m_plot_verticies[i].x =
		  2 * M_PI * static_cast<float>(i) / m_plot_verticies.size() - M_PI;
		m_plot_verticies[i].y = std::sin(m_plot_verticies[i].x);
	}

	glBufferData(GL_ARRAY_BUFFER,
	             sizeof(m_plot_verticies),
	             m_plot_verticies.data(),
	             GL_STATIC_DRAW);

	glVertexAttribPointer(
	  0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
	glEnableVertexAttribArray(0);

	_init_line_buffers();
}

GraphView::~GraphView()
{
	glDeleteBuffers(1, &m_vbo);
	glDeleteVertexArrays(1, &m_vao);
}

void GraphView::_init_line_buffers()
{
	glGenVertexArrays(1, &_linebuf_vao);
	glBindVertexArray(_linebuf_vao);

	glGenBuffers(1, &_linebuf_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, _linebuf_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 128, nullptr, GL_STREAM_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
	glEnableVertexAttribArray(0);

	std::vector<Shader> shaders{ Shader("simple_vertex.glsl", GL_VERTEX_SHADER),
		                         Shader("simple_fragment.glsl",
		                                GL_FRAGMENT_SHADER) };
	_lines_shader = Program(shaders);
}

void GraphView::_draw_lines()
{
	const int margin_px = 60;
	int offset = 0;
	const int TICKLEN = 10;

	auto tick_spacing = _get_tick_spacing();

	glBindVertexArray(_linebuf_vao);

	// Get a pointer to the underlying buffer
	void *raw_ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	auto *ptr = reinterpret_cast<glm::vec2 *>(raw_ptr);

	// Draw the y axis line
	ptr[offset++] = glm::vec2(margin_px, margin_px);
	ptr[offset++] = glm::vec2(margin_px, _size.y - margin_px);

	// Draw the y-axis ticks
	// Work out where (in graph space) margin and height-margin is
	auto top_gs = screen2graph(glm::vec2(margin_px, margin_px));
	auto bottom_gs = screen2graph(glm::vec2(margin_px, _size.y - margin_px));
	float start = ceilf(bottom_gs.y / tick_spacing.y) * tick_spacing.y;
	float end = ceilf(top_gs.y / tick_spacing.y) * tick_spacing.y;

	// Place a tick at every unit up the y axis
	for (float i = start; i < end; i += tick_spacing.y) {
		auto tick_y_vpspace = _viewport_matrix * (_view_matrix * glm::vec3(0.0f, i, 1.0f));
		ptr[offset++] = glm::vec2(margin_px - TICKLEN, tick_y_vpspace.y);
		ptr[offset++] = glm::vec2(margin_px, tick_y_vpspace.y);
	}

	// Draw the x axis line
	ptr[offset++] = glm::vec2(margin_px, _size.y - margin_px);
	ptr[offset++] = glm::vec2(_size.x - margin_px, _size.y - margin_px);

	// Draw the y axis ticks
	auto left_gs = screen2graph(glm::vec2(margin_px, _size.y - margin_px));
	auto right_gs = screen2graph(glm::vec2(_size.x - margin_px, _size.y - margin_px));
	start = ceilf(left_gs.x / tick_spacing.x) * tick_spacing.x;
	end = ceilf(right_gs.x / tick_spacing.x) * tick_spacing.x;

	// Place a tick at every unit along the x axis
	for (float i = start; i < end; i += tick_spacing.x) {
		auto tick_x_vpspace = _viewport_matrix * (_view_matrix * glm::vec3(i, 0.0f, 1.0f));
		ptr[offset++] = glm::vec2(tick_x_vpspace.x, _size.y - margin_px + TICKLEN);
		ptr[offset++] = glm::vec2(tick_x_vpspace.x, _size.y - margin_px);
	}

	// make sure to tell OpenGL we're done with the pointer
	glUnmapBuffer(GL_ARRAY_BUFFER);

	_lines_shader.use();
	int uniform_id = _lines_shader.get_uniform_location("view_matrix");
	glUniformMatrix3fv(uniform_id, 1, GL_FALSE, glm::value_ptr(_viewport_matrix_inv[0]));
	glDrawArrays(GL_LINES, 0, offset);
}

glm::vec2 GraphView::_get_tick_spacing() const
{
	glm::vec3 a(0.0f, 0.0f, 1.0f);
	glm::vec3 b(1.0f, 1.0f, 1.0f);

	auto a_ss = _view_matrix * a;
	auto b_ss = _view_matrix * b;

	auto delta = b_ss - a_ss;

	const float ONSCREEN_TICKS = 0.2f;
	float tick_spacing = (ONSCREEN_TICKS / delta.y);
	tick_spacing = powf(2.0f, floorf(log2f(tick_spacing)));

	return glm::vec2(tick_spacing, tick_spacing);
}
