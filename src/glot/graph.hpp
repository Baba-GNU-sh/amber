#pragma once

#include <glm/glm.hpp>
#include <boost/signals2.hpp>
#include <map>
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

struct MarkerInfo
{
    double *position;
    MarkerStyle style;
    glm::vec3 colour;
    bool is_dragging;
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
     * @brief Reset internal state and initialize the graph with a pointer to a given view matrix.
     */
    void init(glm::dmat3 *view_matrix, const glm::ivec2 &size);

    /**
     * @brief Draw the graph to the screen.
     *
     * @param plot_width
     * @param plot_colour
     * @param minmax_colour
     * @param show_plot_segments
     */
    void draw_plot(const TimeSeries &ts,
                   int plot_width,
                   glm::vec3 plot_colour,
                   float y_offset,
                   bool show_line_segments) const;

    /**
     * @brief Adds a marker to the screen
     *
     * @param label The display name and label used internally to keep track of the marker.
     * @param pos A pointer to the marker's position.
     * @param style The marker's style.
     * @param colour The colour of the marker.
     */
    void draw_marker(const std::string &label,
                     double *position,
                     MarkerStyle style,
                     const glm::vec3 &colour);

  private:
    /**
     * @brief Check if a coodinate is inside a bounding box.
     *
     * @param value The coordinate to test.
     * @param tl Position of the top-left corner of the bounding box.
     * @param br Position of the bottom-right corner of the bounding box.
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
    glm::dvec2 screen2graph(const glm::ivec2 &value) const;

    void init_line_buffers();
    void init_glyph_buffers();
    void draw_lines() const;
    void draw_labels() const;
    void _draw_label(const std::string_view text,
                     const glm::ivec2 &pos,
                     int height,
                     int width,
                     LabelAlignment align,
                     LabelAlignmentVertical valign,
                     const glm::vec3 &colour) const;
    void _draw_glyph(char c, const glm::ivec2 &pos, int height, int width, GlyphData **buf) const;

    std::tuple<glm::dvec2, glm::dvec2, glm::ivec2> tick_spacing() const;

    void on_zoom(double x, double y);
    void update_view_matrix(const glm::dmat3 &new_view_matrix);

    static constexpr double ZOOM_MIN_X = 1'000'000.0;
    static constexpr double ZOOM_MIN_Y = 1'000'000.0;

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

    // Cache variables from the previous render call
    glm::dvec2 m_cursor_old;
    glm::ivec2 m_size;
    bool m_is_dragging;
    glm::dmat3 *m_view_matrix;
    glm::dmat3 m_view_matrix_inv;
    std::map<std::string, MarkerInfo> m_markers;
};
