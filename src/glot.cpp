#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <random>

#include <imgui.h>
#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"

#include "shader_utils.hpp"

#include <random>
#include <iostream>
#include <cmath>

struct Sample
{
    GLfloat t;
    GLfloat avg;
    GLfloat max;
    GLfloat min;
};

/**
 * @brief This guy contains and draws a set of axes and tick markers.
 */
class AxesOverlay
{
public:
    AxesOverlay()
    {
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);

        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        m_verticies[0].x = -1;
        m_verticies[0].y = -1;
        m_verticies[1].x = 1;
        m_verticies[1].y = 1;

        glBufferData(GL_ARRAY_BUFFER, sizeof(m_verticies), m_verticies, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
        glEnableVertexAttribArray(0);

        std::vector<Shader> shaders{
            Shader("simple_vertex.glsl", GL_VERTEX_SHADER),
            Shader("simple_fragment.glsl", GL_FRAGMENT_SHADER)};

        m_program = Program(shaders);
    }

    ~AxesOverlay()
    {
        glDeleteBuffers(1, &m_vbo);
        glDeleteVertexArrays(1, &m_vao);
    }

    void render(glm::vec2 offset, glm::vec2 scale, GLFWwindow *window)
    {
        m_program.use();
        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        int width, height;
        glfwGetWindowSize(window, &width, &height);

        const int margin_px = 30;
        const int ticks_per_unit = 5;
        const int tick_len_px = 10;

        draw_line(
            glm::vec2(margin_px, height - margin_px),
            glm::vec2(width - margin_px, height - margin_px),
            width, height);

        draw_line(
            glm::vec2(margin_px, margin_px),
            glm::vec2(margin_px, height - margin_px),
            width, height);

        for (int i = 0; i < ticks_per_unit; i++) {
            int x = i * (width - margin_px * 2) / (ticks_per_unit - 1);
            x += margin_px;
            draw_line(
                glm::vec2(x, height - margin_px),
                glm::vec2(x, height - (margin_px - tick_len_px)),
                width, height);
        }

        for (int i = 0; i < ticks_per_unit; i++) {
            int y = i * (height - margin_px * 2) / (ticks_per_unit - 1);
            y += margin_px;
            draw_line(
                glm::vec2(margin_px, y),
                glm::vec2(margin_px - tick_len_px, y),
                width, height);
        }
    }

private:
    void draw_line(glm::vec2 tl_win, glm::vec2 tr_win, int width, int height)
    {
        glm::vec2 tl_ss(
            ((2 * tl_win.x) - width) / width,
            ((-2 * tl_win.y) + height) / height
        );

        glm::vec2 tr_ss(
            ((2 * tr_win.x) - width) / width,
            ((-2 * tr_win.y) + height) / height
        );

        m_verticies[0] = tl_ss;
        m_verticies[1] = tr_ss;

        std::cout << m_verticies[0].x << ", " << m_verticies[0].y << '\n';
        std::cout << m_verticies[1].x << ", " << m_verticies[1].y << '\n';

        // Replace the verticies to be the start and end of each line in the list, then draw it
        // This is a really inefficient way of doing this!
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(m_verticies), m_verticies);
        glDrawArrays(GL_LINES, 0, 2);
    }

    GLuint m_vao;
    GLuint m_vbo;
    glm::vec2 m_verticies[2];
    Program m_program;
};

class GraphWindow
{
public:
    GraphWindow()
        : m_cursor(0.0),
          m_offset(0.0),
          m_zoom(1.0),
          m_dragging(false),
          m_time(0)
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
        glEnable(GL_DEPTH_TEST);

        load_shaders();
        init_imgui();


        axes = std::make_shared<AxesOverlay>();
        // axes2 = std::make_shared<AxesOverlay>(false);


        // // Create and bind the Vertex Array Object
        // glGenVertexArrays(1, &m_vertex_array);
        // glBindVertexArray(m_vertex_array);

        // // Create and bind the Vertex Buffer Object
        // glGenBuffers(1, &m_vertex_buffer);
        // glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
        // glBufferData(GL_ARRAY_BUFFER, sizeof(graph), graph, GL_DYNAMIC_DRAW);

        // // Set vertex array attributes such as the element type, and the stride
        // glEnableVertexAttribArray(0);
        // glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
        // glEnableVertexAttribArray(1);
        // glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));

        // // Set the colour to be a nice dark green
        // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);


        // glGenVertexArrays(1, &m_axis_buffer);
        // glBindVertexArray(m_axis_buffer);

        // // Create and bind the Vertex Buffer Object
        // glGenBuffers(1, &m_axis_vertex_buffer);
        // glBindBuffer(GL_ARRAY_BUFFER, m_axis_vertex_buffer);
        // glBufferData(GL_ARRAY_BUFFER, sizeof(m_axis_points), m_axis_points, GL_STATIC_DRAW);

        // glEnableVertexAttribArray(0);
        // glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);

        // m_axis_points[0] = glm::vec2(-1, -1);
        // m_axis_points[1] = glm::vec2(-1, 1);
        // m_axis_points[2] = glm::vec2(1, 1);
        // m_axis_points[3] = glm::vec2(1, -1);
    }

    std::shared_ptr<AxesOverlay> axes;

    GLuint m_axis_buffer, m_axis_vertex_buffer;
    glm::vec2 m_axis_points[4];

    ~GraphWindow()
    {
        // glDeleteVertexArrays(1, &m_vertex_array);
        // glDeleteBuffers(1, &m_vertex_buffer);
        glfwDestroyWindow(m_window);
    }

    void draw_axes()
    {
        return;

        // Draw lines using the VBO data to the backbuffer then swap buffers
        glBindVertexArray(m_axis_buffer);
        glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, NPOINTS);
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
        //ImGui::StyleColorsClassic();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(m_window, true);

        const char* glsl_version = "#version 130";
        ImGui_ImplOpenGL3_Init(glsl_version);
    }

    void plot_graph()
    {
        std::random_device dev;
        std::mt19937 rng(dev());
        std::uniform_real_distribution<> dist(-1, 1);

        for (int i = 0; i < NPOINTS; i++)
        {
            float x = (i - 1000.0) / 200.0;
            graph[i].t = x;
            graph[i].avg = std::sin(m_time + x * 10.0) / (1.0 + x * x) + 0.01 * dist(rng);
            graph[i].max = graph[i].avg + 0.1 + 0.05 * dist(rng);
            graph[i].min = graph[i].avg - 0.1 + 0.05 * dist(rng);
        }
        m_time += 0.01;
    }

    void render_imgui()
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Frame times");

        ImGui::Checkbox("sin(t)", &sint);      // Edit bools storing our window open/close state

        // ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        // ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        // if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
        //     counter++;
        // ImGui::SameLine();
        // ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.1f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();

        ImGui::Render();
    }

    void render()
    {
        // Generate an interesting function
        plot_graph();


        // Clear the buffer with a nice dark blue/green colour
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        render_imgui();

        axes->render(glm::vec2(0, 0), glm::vec2(1, 1), m_window);

        // draw_axes();

        // m_shader_program.use();
        // glBindVertexArray(m_vertex_array);
        // glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);

        

        // // Swap the data in graph into the VBO
        // glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(graph), graph);

        // // Draw lines using the VBO data to the backbuffer then swap buffers
        // glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0, NPOINTS);



        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(m_window);
    }

    void spin()
    {
        while (!glfwWindowShouldClose(m_window))
        {
            process_input();
            render();
            glfwPollEvents();
        }
    }

private:
    static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
    {
        std::cout << "Resized window " << width << "x" << height << "px\n";
        glViewport(0, 0, width, height);

        GraphWindow *obj = static_cast<GraphWindow *>(glfwGetWindowUserPointer(window));
        glUniform2i(obj->m_uniform_viewport_res, width, height);
    }

    static void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos)
    {
        glm::vec2 cursor(xpos, ypos);

        GraphWindow *obj = static_cast<GraphWindow *>(glfwGetWindowUserPointer(window));
        if (obj->m_dragging)
        {
            auto offset = cursor - obj->m_cursor;
            obj->m_offset += obj->win2screen(offset);

            std::cout << "Dragging: " << obj->m_offset.x << ", " << obj->m_offset.y << "\n";

            glUniform2f(obj->m_uniform_offset, obj->m_offset.x, obj->m_offset.y);
        }

        obj->m_cursor = cursor;
    }

    static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
    {
        std::cout << "Scrolling: " << yoffset << '\n';

        GraphWindow *obj = static_cast<GraphWindow *>(glfwGetWindowUserPointer(window));
        obj->m_zoom *= 1.0 + (yoffset / 10);

        glUniform2f(obj->m_uniform_scale, obj->m_zoom, obj->m_zoom);
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

    void load_shaders()
    {
        std::vector<Shader> shaders{
            Shader("vertex.glsl", GL_VERTEX_SHADER),
            Shader("geometry.glsl", GL_GEOMETRY_SHADER),
            Shader("fragment.glsl", GL_FRAGMENT_SHADER)};

        m_shader_program = Program(shaders);

        m_uniform_offset = m_shader_program.get_uniform_location("offset");
        m_uniform_scale = m_shader_program.get_uniform_location("scale");
        m_uniform_viewport_res = m_shader_program.get_uniform_location("viewport_res_px");
        m_uniform_line_thickness = m_shader_program.get_uniform_location("line_thickness_px");

        m_shader_program.use();

        // Set up uniform initial values
        glUniform2f(m_uniform_scale, m_zoom, m_zoom);
        glUniform2f(m_uniform_offset, m_offset.x, m_offset.y);
        glUniform1f(m_uniform_line_thickness, LINE_THICKNESS_PX);

        // Set up the line window size uniform
        int width, height;
        glfwGetWindowSize(m_window, &width, &height);
        glUniform2i(m_uniform_viewport_res, width, height);
    }

    /**
     * @brief Converts from window coordinates )e.g. pixels) to screen space.
     * Screen space is -1->1 in both axis with the origin in the bottom left.
     *
     * @param win Windows coordinates in px.
     * @return glm::vec2 The screen space vector.
     */
    glm::vec2 win2screen(const glm::vec2 &win)
    {
        int width, height;
        glfwGetWindowSize(m_window, &width, &height);

        glm::vec2 ret;
        ret.x = 2 * win.x / width;
        ret.y = -2 * win.y / height;

        return ret;
    }

    static constexpr unsigned int SCR_WIDTH = 800;
    static constexpr unsigned int SCR_HEIGHT = 600;
    static constexpr std::size_t NPOINTS = 2000;
    static constexpr int LINE_THICKNESS_PX = 4;

    GLFWwindow *m_window;
    Program m_shader_program;
    int m_uniform_offset, m_uniform_scale, m_uniform_viewport_res, m_uniform_line_thickness;
    unsigned int m_vertex_array, m_vertex_buffer;
    glm::vec2 m_cursor;
    glm::vec2 m_offset;
    double m_zoom;
    Sample graph[NPOINTS];
    bool m_dragging;
    float m_time;
    bool sint;
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
