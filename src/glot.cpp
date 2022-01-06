#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "shader_utils.hpp"
#include "tsdb/tsdb.hpp"

#include <random>
#include <iostream>
#include <cmath>

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

        glEnable(GL_MULTISAMPLE);

        load_shaders();

        glGenVertexArrays(1, &m_vertex_array);
        // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
        glBindVertexArray(m_vertex_array);

        glGenBuffers(1, &m_vertex_buffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(graph), graph, GL_DYNAMIC_DRAW);

        // vertices[0] = -1.0f;
        // glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);
        glEnableVertexAttribArray(0);

        // note that this is allowed, the call to glVertexAtribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // You can unbind the vertex_array afterwards so other vertex_array calls won't accidentally modify this vertex_array, but this rarely happens. Modifying other
        // vertex_arrays requires a call to glBindVertexArray anyways so we generally don't unbind vertex_arrays (nor VBOs) when it's not directly necessary.
        glBindVertexArray(0);

        // uncomment this call to draw in wireframe polygons.
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        for (int i = 0; i < 3; i++)
        {
            graph[i].x = i - 1.0;
            graph[i].y = std::sin(static_cast<double>(i));
        }
    }

    ~GraphWindow()
    {
        // optional: de-allocate all resources once they've outlived their purpose:
        // ------------------------------------------------------------------------
        glDeleteVertexArrays(1, &m_vertex_array);
        glDeleteBuffers(1, &m_vertex_buffer);

        glfwDestroyWindow(m_window);
    }

    void render()
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(m_shader_program->get_handle());

        int width, height;
        glfwGetWindowSize(m_window, &width, &height);

        // Feed line thickness info to the geometry shader, this depends on the window size
        const auto lt = win2screen(glm::vec2(LINE_THICKNESS_PX, -LINE_THICKNESS_PX));
        glUniform2f(m_uniform_line_thickness, lt.x, lt.y);

        // Feed offset and scale info to the shaders
        glUniform2f(m_uniform_offset, m_offset.x, m_offset.y);
        glUniform2f(m_uniform_scale, m_zoom, m_zoom);

        // seeing as we only have a single vertex_array there's no need to bind it every time, but we'll do so to keep things a bit more organized
        glBindVertexArray(m_vertex_array);

        // Plot an interesting function
        for (int i = 0; i < NPOINTS; i++)
        {
            float x = (i - 1000.0) / 100.0;
            graph[i].x = x;
            graph[i].y = std::sin(m_time * 0.01 + x * 10.0) / (1.0 + x * x);
        }
        m_time += 1.0;

        glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(graph), graph);

        glDrawArrays(GL_LINE_STRIP, 0, NPOINTS);
        glBindVertexArray(0);

        glfwSwapBuffers(m_window);
    }

    void spin()
    {
        // render loop
        // -----------
        while (!glfwWindowShouldClose(m_window))
        {
            process_input();
            render();
            glfwPollEvents();
        }
    }

private:
    static void framebuffer_size_callback(GLFWwindow *, int width, int height)
    {
        std::cout << "Resized window " << width << "x" << height << "px\n";
        glViewport(0, 0, width, height);
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
        }

        obj->m_cursor = cursor;
    }

    static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
    {
        std::cout << "Scrolling: " << yoffset << '\n';

        GraphWindow *obj = static_cast<GraphWindow *>(glfwGetWindowUserPointer(window));
        obj->m_zoom *= 1.0 + (yoffset / 10);
    }

    static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
    {
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
        std::vector<std::shared_ptr<Shader>> shaders{
            std::make_shared<Shader>("shaders/vertex.glsl", GL_VERTEX_SHADER),
            std::make_shared<Shader>("shaders/geometry.glsl", GL_GEOMETRY_SHADER),
            std::make_shared<Shader>("shaders/fragment.glsl", GL_FRAGMENT_SHADER)};

        m_shader_program = std::make_unique<ShaderProgram>(std::move(shaders));

        m_uniform_offset = m_shader_program->get_uniform_location("offset");
        m_uniform_scale = m_shader_program->get_uniform_location("scale");
        m_uniform_line_thickness = m_shader_program->get_uniform_location("line_thickness");
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
    static constexpr int LINE_THICKNESS_PX = 5.0;

    GLFWwindow *m_window;
    std::unique_ptr<ShaderProgram> m_shader_program;
    int m_uniform_offset, m_uniform_scale, m_uniform_line_thickness;
    unsigned int m_vertex_array, m_vertex_buffer;
    glm::vec2 m_cursor;
    glm::vec2 m_offset;
    double m_zoom;
    glm::vec2 graph[NPOINTS];
    bool m_dragging;
    float m_time;
};

// settings

int main()
{
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
        win.spin();
    }

    // std::random_device dev;
    // std::mt19937 rng(dev());

    // std::uniform_real_distribution<> dist(-.3, .3);

    glfwTerminate();

    std::cout << "Bye!\n";

    return 0;
}
