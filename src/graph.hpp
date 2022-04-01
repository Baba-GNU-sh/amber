#pragma once

#include <glad/glad.h> // Keep this one before glfw to avoid errors

#include <GLFW/glfw3.h>

#include <glm/fwd.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>

#include "plot.hpp"
#include "shader_utils.hpp"

// #include "font.hpp"
#include <glm/glm.hpp>
#include "database.hpp"

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
    void set_size(int width, int height);

    /**
     * @brief Get the size of the graph in the viewport.
     *
     * @return glm::vec2 2D vector containing the size of the graph in pixels.
     */
    glm::ivec2 size() const;

    /**
     * @brief Update the position of the graph in the viewport.
     *
     * @param x The new x position in pixels.
     * @param y The new y position in pixels.
     */
    void set_position(int x, int y);

    /**
     * @brief Get the posiiton of the graph in the viewport.
     *
     * @return glm::vec2 2D vector containing the posiiton of the graph in
     * pixels.
     */
    glm::ivec2 position() const;

    /**
     * @brief Call this to pass mouse button events through from GLFW to control
     * the graph.
     */
    void mouse_button(int button, int action, int mods);

    /**
     * @brief Call this to pass through cursor move events from GLFW to control
     * the graph.
     */
    void cursor_move(double xpos, double ypos);

    /**
     * @brief Call this to pass through mouse scroll events from GLFW to control
     * the graph.
     */
    void mouse_scroll(double xoffset, double yoffset);

    /**
     * @brief Update the matrix describing the transform from screen space to
     * pixels.
     *
     * @param viewport_matrix The new value of the viewport matrix.
     */
    void update_viewport_matrix(const glm::mat3x3 &viewport_matrix);

    void draw() const;

    glm::mat3x3 get_view_matrix() const;

    /**
     * @brief Get the cursor's position in graph space.
     *
     * @return glm::vec2
     */
    glm::vec2 get_cursor_graphspace() const;

    int *get_plot_thickness();

    glm::vec3 *get_plot_colour();

    glm::vec3 *get_minmax_colour();

    bool *get_show_line_segments();

    void set_database(const Database &db);

  private:
    /**
     * @brief Check if a coodinate is inside a bounding box defined by two corners.
     *
     * @param value The value to test.
     * @param tl Top-left coordinate of the bounding box.
     * @param br Bottom-right coordinate of the bounding box.
     * @return true The coordinate is within the bounding box.
     * @return false The coordinate is outside the bounding box.
     */
    bool _hittest(glm::vec2 value, glm::vec2 tl, glm::vec2 br);

    /**
     * @brief Converts a vector from viewport space (pixels w/ origin at TL) to
     * graph space.
     *
     * @param value The vector to convert.
     * @return glm::vec2 The resultant vector in graph space.
     */
    glm::vec2 screen2graph(const glm::ivec2 &value) const;

    void _update_view_matrix(const glm::mat3x3 &value);

    void _init_line_buffers();
    void _init_glyph_buffers();
    void _draw_lines() const;
    void _draw_labels() const;
    void _draw_label(const std::string_view text, const glm::vec2 &pos, float height, float width, LabelAlignment align,
                     LabelAlignmentVertical valign) const;
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
    const int TICKLEN = 8;

    Plot _plot;

    std::shared_ptr<TimeSeries> _ts;
};