#pragma once

#include <glm/glm.hpp>
#include "plot_renderer_opengl.hpp"
#include "marker_renderer_opengl.hpp"
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

/**
 * @brief The GraphRendererOpenGL is responsible for rendering the graph using OpenGL calls.
 *
 * Note: This renderer requires that depth testing and alpha blending are both enabled in order to
 * work properly.
 */
class GraphRendererOpenGL
{
  public:
    /**
     * @param window The window we shall be rendering to.
     */
    GraphRendererOpenGL(Window &window);
    ~GraphRendererOpenGL();

    /**
     * @brief Set the transformation between graphspace and clipspace.
     */
    void set_view_matrix(const glm::dmat3 &view_matrix);

    /**
     * @brief Set the size of the graph in the viewport in pixels.
     */
    void set_size(const glm::ivec2 &size);

    /**
     * @brief Set the size of the spacing between the edge of the graph and the plot area in pixels.
     */
    void set_gutter_size(int gutter_size_px);

    /**
     * @brief Set the length of the ticks in pixels.
     */
    void set_tick_len(int tick_len_px);

    /**
     * @brief Draws the graph axes, ticks and labels. Doesn't actually draw any plots.
     */
    void draw_graph() const;

    /**
     * @brief Draws a timeseries onto the graph.
     */
    void draw_plot(const TimeSeries &ts,
                   int plot_width,
                   glm::vec3 plot_colour,
                   float y_offset,
                   bool show_line_segments) const;

    /**
     * @brief Adds a marker to the graph.
     *
     * @param label The display name of the marker.
     * @param position The marker's position.
     * @param style The marker's style.
     * @param colour The colour of the marker.
     */
    void draw_marker(const std::string &label,
                     double position,
                     MarkerStyle style,
                     const glm::vec3 &colour) const;

  private:
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

    /**
     * @brief Converts a vector from viewport space (pixels w/ origin at TL) to
     * graph space.
     *
     * @param value The vector to convert.
     * @return glm::dvec2 The resultant vector in graph space.
     */
    glm::dvec2 screen2graph(const glm::ivec2 &value) const;

    Window &m_window;
    PlotRendererOpenGL m_plot;
    MarkerRendererOpenGL m_marker_renderer;

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

    // Variants
    glm::dmat3 m_view_matrix;
    glm::dmat3 m_view_matrix_inv;
    glm::ivec2 m_size;
    int m_gutter_size_px;
    int m_tick_len_px;
};
