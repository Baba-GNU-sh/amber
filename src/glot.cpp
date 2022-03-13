#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>

#include <glm/matrix.hpp>
#include <imgui.h>
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"

#include "shader_utils.hpp"

#include <random>
#include <iostream>
#include <cmath>
#include <stdexcept>

#include "font.hpp"

struct Sample
{
    GLfloat t;
    GLfloat avg;
    GLfloat max;
    GLfloat min;
};

/**
 * @brief Draws the graph.
 */
class GraphView
{
public:
    GraphView(float tick_spacing = 0.1)
        : m_tick_spacing(tick_spacing)
    {
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        glBufferData(GL_ARRAY_BUFFER, sizeof(m_verticies), m_verticies, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
        glEnableVertexAttribArray(0);

        std::vector<Shader> shaders{
            Shader("simple_vertex.glsl", GL_VERTEX_SHADER),
            Shader("simple_fragment.glsl", GL_FRAGMENT_SHADER)};

        m_program = Program(shaders);
        m_uniform_view_matrix = m_program.get_uniform_location("view_matrix");

        // Generate buffers for the actual plot
        glGenVertexArrays(1, &m_plot_vao);
        glBindVertexArray(m_plot_vao);

        glGenBuffers(1, &m_plot_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_plot_vbo);

        for (int i = 0; i < m_plot_verticies.size(); i++)
        {
            m_plot_verticies[i].x = 2 * M_PI * static_cast<float>(i) / m_plot_verticies.size() - M_PI;
            m_plot_verticies[i].y = std::sin(m_plot_verticies[i].x);
        }

        glBufferData(GL_ARRAY_BUFFER, sizeof(m_plot_verticies), m_plot_verticies.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
        glEnableVertexAttribArray(0);
    }

    ~GraphView()
    {
        glDeleteBuffers(1, &m_vbo);
        glDeleteVertexArrays(1, &m_vao);
    }

    void draw(glm::mat3x3 view_matrix, glm::mat3x3 viewport_matrix, GLFWwindow *window)
    {
        m_program.use();
        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        glm::mat3 indent(1.0);
        glUniformMatrix3fv(m_uniform_view_matrix, 1, GL_FALSE, glm::value_ptr(indent[0]));

        // // Draw something in the screen, just to have something to look at
        // draw_line(view_matrix, glm::vec2(-.5f, -.5f), glm::vec2(-.5f, .5f));
        // draw_line(view_matrix, glm::vec2(.5f, -.5f), glm::vec2(.5f, .5f));

        auto viewport_matrix_inv = glm::inverse(viewport_matrix);
        auto view_matrix_inv = glm::inverse(view_matrix);

        int margin_px = 60;
        const int TICKLEN = 6;
        const int TEXT_SPACING = 30;
        int width, height;
        glfwGetWindowSize(window, &width, &height);

        {
            // draw a line from margin_px to height - margin_px
            glm::vec2 start_px(margin_px, margin_px);
            glm::vec2 end_px(margin_px, height - margin_px);
            auto start_clipspace = viewport_matrix_inv * glm::vec3(start_px, 1.0);;
            auto end_clipspace = viewport_matrix_inv * glm::vec3(end_px, 1.0);
            draw_line_clipspace(start_clipspace, end_clipspace);

            // Work out where (in graph space) margin and height-margin is
            auto top_gs = view_matrix_inv * (viewport_matrix_inv * glm::vec3(0, margin_px, 1.0));
            auto bottom_gs = view_matrix_inv * (viewport_matrix_inv * glm::vec3(0, height - margin_px, 1.0));
            auto start = (bottom_gs.y > 0)? static_cast<int>(bottom_gs.y) + 1 : static_cast<int>(bottom_gs.y);

            // Place a tick at every unit up the y axis
            for (int i = start; i < top_gs.y; i++) {
                m_program.use();
                glBindVertexArray(m_vao);
                glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

                auto tick_y_vpspace = viewport_matrix * (view_matrix * glm::vec3(0.0f, static_cast<float>(i), 1.0f));
                auto tick_start = viewport_matrix_inv * glm::vec3(margin_px - TICKLEN, tick_y_vpspace.y, 1);
                auto tick_end = viewport_matrix_inv * glm::vec3(margin_px, tick_y_vpspace.y, 1);
                draw_line_clipspace(tick_start, tick_end);

                char buf[16];
                std::snprintf(buf, 15, "%.1f", static_cast<float>(i));

                // Glyphs should be 8*16 pixels
                glm::vec2 text_position(margin_px - TICKLEN - TEXT_SPACING, tick_y_vpspace.y);
                glm::vec2 glyph_size(8.0f, -8.0f);

                auto text_position_tl = viewport_matrix_inv * glm::vec3(text_position, 1.0);
                auto text_position_tr = viewport_matrix_inv * glm::vec3((text_position + glyph_size), 1.0);

                auto scale = text_position_tr - text_position_tl;

                glm::mat3x3 m(1.0);
                m = glm::translate(m, glm::vec2(text_position_tl.x, text_position_tl.y));
                m = glm::scale(m, glm::vec2(scale.x, scale.y));
                text.draw_text(buf, m);
            }
        }

        {
            m_program.use();
            glBindVertexArray(m_vao);
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

            // draw a line from margin_px to height - margin_px
            glm::vec2 start_px(margin_px, height - margin_px);
            glm::vec2 end_px(width - margin_px, height - margin_px);
            auto start_clipspace = viewport_matrix_inv * glm::vec3(start_px, 1.0);;
            auto end_clipspace = viewport_matrix_inv * glm::vec3(end_px, 1.0);
            draw_line_clipspace(start_clipspace, end_clipspace);

            auto left_gs = view_matrix_inv * (viewport_matrix_inv * glm::vec3(margin_px, 0.0f, 1.0f));
            auto right_gs = view_matrix_inv * (viewport_matrix_inv * glm::vec3(width - margin_px, 0.0f, 1.0f));
            auto start = (left_gs.x > 0)? static_cast<int>(left_gs.x) + 1 : static_cast<int>(left_gs.x);

            // Place a tick at every unit up the y axis
            for (int i = start; i < right_gs.x; i++) {
                m_program.use();
                glBindVertexArray(m_vao);
                glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

                auto tick_x_vpspace = viewport_matrix * (view_matrix * glm::vec3(static_cast<float>(i), 0.0f, 1.0f));
                auto tick_start = viewport_matrix_inv * glm::vec3(tick_x_vpspace.x, height - (margin_px - TICKLEN), 1);
                auto tick_end = viewport_matrix_inv * glm::vec3(tick_x_vpspace.x, height - margin_px, 1);
                draw_line_clipspace(tick_start, tick_end);

                char buf[16];
                std::snprintf(buf, 15, "%.1f", static_cast<float>(i));

                // Glyphs should be 8*16 pixels
                glm::vec2 text_position(tick_x_vpspace.x, (height - margin_px) + TEXT_SPACING);
                glm::vec2 glyph_size(8.0f, -8.0f);

                auto text_position_tl = viewport_matrix_inv * glm::vec3(text_position, 1.0);
                auto text_position_tr = viewport_matrix_inv * glm::vec3((text_position + glyph_size), 1.0);

                auto scale = text_position_tr - text_position_tl;

                glm::mat3x3 m(1.0);
                m = glm::translate(m, glm::vec2(text_position_tl.x, text_position_tl.y));
                m = glm::scale(m, glm::vec2(scale.x, scale.y));
                text.draw_text(buf, m);
            }
        }

        m_program.use();
        glUniformMatrix3fv(m_uniform_view_matrix, 1, GL_FALSE, glm::value_ptr(view_matrix[0]));

        glBindVertexArray(m_plot_vao);
        glDrawArrays(GL_LINE_STRIP, 0, 1000);
    }

private:
    void draw_line_clipspace(glm::vec2 start, glm::vec2 end)
    {
        m_verticies[0] = start;
        m_verticies[1] = end;

        // Replace the verticies to be the start and end of each line in the list, then draw it
        // This is a really inefficient way of doing this!
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(m_verticies), m_verticies);
        glDrawArrays(GL_LINES, 0, 2);
    }

    void draw_line(glm::mat3x3 viewmat, glm::vec2 start, glm::vec2 end)
    {
        m_verticies[0] = viewmat * glm::vec3(start, 1.0);
        m_verticies[1] = viewmat * glm::vec3(end, 1.0);

        // Replace the verticies to be the start and end of each line in the list, then draw it
        // This is a really inefficient way of doing this!
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(m_verticies), m_verticies);
        glDrawArrays(GL_LINES, 0, 2);
    }

    float m_tick_spacing;
    Program m_program;
    GLuint m_vao;
    GLuint m_vbo;
    glm::vec2 m_verticies[2];

    GLuint m_plot_vao, m_plot_vbo;
    std::array<glm::vec2, 1000> m_plot_verticies;
    GLint m_uniform_view_matrix;

    int width, height, nrChannels;
    unsigned char *m_tex_data;
    unsigned int texture;

    TextRenderer text;
};

class GraphWindow
{
public:
    GraphWindow()
        : m_cursor(0.0),
          m_dragging(false),
          m_viewmat(1.0f), // Identity matrix
          m_viewportmat(1.0f)
    {
        m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GLOT", NULL, NULL);
        if (m_window == NULL)
        {
            glfwTerminate();
            throw std::runtime_error("Failed to create GLFW window");
        }

        glfwSetWindowUserPointer(m_window, this);
        glfwMakeContextCurrent(m_window);
        glfwSetFramebufferSizeCallback(m_window, GraphWindow::framebuffer_size_callback);
        glfwSetCursorPosCallback(m_window, GraphWindow::cursor_pos_callback);
        glfwSetScrollCallback(m_window, GraphWindow::scroll_callback);
        glfwSetMouseButtonCallback(m_window, GraphWindow::mouse_button_callback);

        // glad: load all OpenGL function pointers
        // ---------------------------------------
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            throw std::runtime_error("Failed to initialize GLAD");
        }

        // Optimistically attempt to enable multisampling
        glEnable(GL_MULTISAMPLE);

        // Depths test helps us with the rendering for a small perf penalty
        glEnable(GL_DEPTH_TEST);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);

        // Set the colour to be a nice dark green
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

        // Initialize the scene
        init_imgui();

        m_graph = std::make_shared<GraphView>();

        update_viewport_matrix(SCR_WIDTH, SCR_HEIGHT);
    }

    ~GraphWindow()
    {
        glfwDestroyWindow(m_window);
    }

    void update_viewport_matrix(int width, int height)
    {
        m_viewportmat = glm::scale(glm::mat3x3(1.0f), glm::vec2(width / 2, -height / 2));
        m_viewportmat = glm::translate(m_viewportmat, glm::vec2(1, -1));
    }

    void init_imgui()
    {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        // io.ImGuiConfigFlags_WantCaptureMouse |= ImGuiConfigFlags_WantCaptureMouse;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);

        const char* glsl_version = "#version 130";
        ImGui_ImplOpenGL3_Init(glsl_version);
    }

    void draw()
    {
        // Clear the buffer with a nice dark blue/green colour
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        m_graph->draw(m_viewmat, m_viewportmat, m_window);

        render_imgui();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(m_window);
    }

    void render_imgui()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Frame times");
        ImGui::Text("Application average %.1f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("ViewMat:");
        for (int i = 0; i < 3; i++)
        {
            ImGui::Text("%f %f %f", m_viewmat[0][i], m_viewmat[1][i], m_viewmat[2][i]);
        }

        auto cursor_gs = viewport2graph(m_cursor);
        ImGui::Text("Cursor: %f %f", cursor_gs.x, cursor_gs.y);
        ImGui::End();

        ImGui::Render();
    }

    void spin()
    {
        while (!glfwWindowShouldClose(m_window))
        {
            process_input();
            draw();
            glfwPollEvents();
        }
    }

private:
    static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
    {
        std::cout << "Resized window " << width << "x" << height << "px\n";
        glViewport(0, 0, width, height);
        GraphWindow *obj = static_cast<GraphWindow *>(glfwGetWindowUserPointer(window));
        obj->update_viewport_matrix(width, height);
    }

    static void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos)
    {
        glm::vec2 cursor(xpos, ypos);

        GraphWindow *win = static_cast<GraphWindow *>(glfwGetWindowUserPointer(window));
        if (win->m_dragging)
        {
            auto offset = cursor - win->m_cursor;
            std::cout << "Dragging: " << offset.x << ", " << offset.y << "\n";

            auto cursor_gs_old = win->viewport2graph(win->m_cursor);
            auto cursor_gs_new = win->viewport2graph(cursor);
            auto cursor_gs_delta = cursor_gs_new - cursor_gs_old;

            win->m_viewmat = glm::translate(win->m_viewmat, cursor_gs_delta);
        }

        win->m_cursor = cursor;
    }

    static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
    {
        std::cout << "Scrolling: " << yoffset << '\n';
        GraphWindow *win = static_cast<GraphWindow *>(glfwGetWindowUserPointer(window));

        // Work out where the pointer is in graph space
        auto cursor_in_gs_old = win->viewport2graph(win->m_cursor);
        float zoom_delta = 1.0 + (yoffset / 10);
        win->m_viewmat = glm::scale(win->m_viewmat, glm::vec2(zoom_delta, zoom_delta));
        auto cursor_in_gs_new = win->viewport2graph(win->m_cursor);
        auto cursor_delta = cursor_in_gs_new - cursor_in_gs_old;

        win->m_viewmat = glm::translate(win->m_viewmat, cursor_delta);
    }

    static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
    {
        ImGuiIO& io = ImGui::GetIO();
        bool capture = io.WantCaptureMouse;
        if (capture) return;

        GraphWindow *obj = static_cast<GraphWindow *>(glfwGetWindowUserPointer(window));
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        {
            obj->m_dragging = true;
        }
        else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
        {
            obj->m_dragging = false;
        }
    }

    void process_input()
    {
        if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(m_window, true);
        }
    }

    glm::vec2 viewport2graph(const glm::vec2 &viewport_coord)
    {
        auto viewportmat_inv = glm::inverse(m_viewportmat);
        auto clipspace_coord = viewportmat_inv * glm::vec3(viewport_coord, 1.0f);

        auto viewmat_inv = glm::inverse(m_viewmat);
        auto graphspace_coord = viewmat_inv * clipspace_coord;

        return graphspace_coord;
    }

    static constexpr unsigned int SCR_WIDTH = 800;
    static constexpr unsigned int SCR_HEIGHT = 600;
    static constexpr std::size_t NPOINTS = 2000;
    static constexpr int LINE_THICKNESS_PX = 4;

    GLFWwindow *m_window;
    glm::vec2 m_cursor;
    bool m_dragging;
    glm::mat3x3 m_viewmat; // Converts from graph-space to clip-space
    glm::mat3x3 m_viewportmat; // Converts from clip-space to viewport-space
    std::shared_ptr<GraphView> m_graph;
};

void error_callback(int error, const char *msg)
{
    std::string s;
    s = " [" + std::to_string(error) + "] " + msg + '\n';
    std::cerr << s << std::endl;
}

int main()
{
    glfwSetErrorCallback(error_callback);

    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    {
        GraphWindow win;
        std::cout << "Use the scroll wheel to zoom in and out, and use the left mouse button to drag the canvas around\n";
        win.spin();
    }

    glfwTerminate();

    std::cout << "Bye!\n";

    return 0;
}
