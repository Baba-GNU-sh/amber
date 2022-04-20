#pragma once

#include <glm/glm.hpp>
#include <boost/signals2.hpp>
#include "plot.hpp"
#include "shader_utils.hpp"
#include "database.hpp"
#include "timeseries.hpp"
#include "window.hpp"

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

enum class MarkerStyle
{
    Left,
    Right,
    Standalone
};

struct TimeSeriesContainer
{
    std::shared_ptr<TimeSeries> ts;
    glm::vec3 colour;
    std::string name;
    bool visible;
    float y_offset;
};

/**
 * @brief Stores and renders a graph with axes and zoom and pan mouse controls.
 */
class Graph
{
  public:
    Graph(Window &window, int gutter_size_px = 60, int tick_len_px = 5);
    ~Graph();

    /**
     * @brief Update the size of the graph in the viewport.
     *
     * @param width The new width of the graph view in pixels.
     * @param height The new height of the graph view in pixels.
     */
    void set_size(const glm::ivec2 &size);

    /**
     * @brief Get the size of the graph in the pixels.
     */
    glm::ivec2 size() const;

    /**
     * @brief Update the position of the graph in the viewport.
     *
     * @param x The new x position in pixels.
     * @param y The new y position in pixels.
     */
    void set_position(const glm::ivec2 &position);

    /**
     * @brief Get the posiiton of the graph in the viewport.
     */
    glm::ivec2 position() const;

    /**
     * @brief Call this to pass mouse button events through from GLFW to control
     * the graph.
     */
    void mouse_button(int button, int action, int mods);

    /**
     * @brief Draw the graph to the screen.
     *
     * @param plot_width
     * @param plot_colour
     * @param minmax_colour
     * @param show_plot_segments
     */
    void draw_decorations(const glm::dmat3 &view_matrix) const;

    void draw_plot(const glm::mat3 &view_matrix,
                   const TimeSeries &ts,
                   int plot_width,
                   glm::vec3 plot_colour,
                   float y_offset,
                   bool show_line_segments) const;

    void draw_marker(const glm::mat3 &view_matrix,
                     double marker_x_pos,
                     MarkerStyle style,
                     const glm::vec3 &colour) const;

    boost::signals2::signal<void(double, double)> on_drag;
    boost::signals2::signal<void(double, double)> on_zoom;

  private:
    void on_cursor_move(double xpos, double ypos);
    void on_mouse_scroll(double xoffset, double yoffset);

    /**
     * @brief Check if a coodinate is inside a bounding box defined by two corners.
     *
     * @param value The value to test.
     * @param tl Top-left coordinate of the bounding box.
     * @param br Bottom-right coordinate of the bounding box.
     * @return true The coordinate is within the bounding box.
     * @return false The coordinate is outside the bounding box.
     */
    static bool hit_test(glm::ivec2 value, glm::ivec2 tl, glm::ivec2 br);

    /**
     * @brief Converts a vector from viewport space (pixels w/ origin at TL) to
     * graph space.
     *
     * @param value The vector to convert.
     * @return glm::dvec2 The resultant vector in graph space.
     */
    glm::dvec2 screen2graph(const glm::dmat3 &view_matrix, const glm::ivec2 &value) const;

    void _init_line_buffers();
    void _init_glyph_buffers();
    void _draw_lines(const glm::dmat3 &view_matrix) const;
    void _draw_labels(const glm::dmat3 &view_matrix) const;
    void _draw_label(const std::string_view text,
                     const glm::ivec2 &pos,
                     int height,
                     int width,
                     LabelAlignment align,
                     LabelAlignmentVertical valign,
                     const glm::vec3 &colour) const;
    void _draw_glyph(char c, const glm::ivec2 &pos, int height, int width, GlyphData **buf) const;

    std::tuple<glm::dvec2, glm::dvec2, glm::ivec2> _tick_spacing(
        const glm::dmat3 &view_matrix) const;

    Window &m_window;
    const int m_gutter_size_px;
    const int m_tick_len_px;
    Plot m_plot;

    // Line buffers
    GLuint _linebuf_vao;
    GLuint _linebuf_vbo;
    Program _lines_shader;

    // Glyph buffers
    GLuint _glyphbuf_vao;
    GLuint _glyphbuf_vbo;
    GLuint _glyphbuf_ebo;
    Program _glyph_shader;
    GLuint _glyph_texture;

    glm::ivec2 _position;
    glm::ivec2 _size;
    glm::dvec2 _cursor;

    bool _dragging;
};
