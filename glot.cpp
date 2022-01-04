#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "shader_loader.hpp"
#include "tsdb/tsdb.hpp"

#include <random>
#include <iostream>
#include <cmath>

struct point
{
    GLfloat x;
    GLfloat y;
};

class GraphWindow
{
public:
    GraphWindow()
        : m_cursor(0.0),
          m_zoom(1.0)
    {
        m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
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

        for (int i = 0; i < 2000; i++)
        {
            graph[i].x = i;
            graph[i].y = std::sin(static_cast<double>(i));
        }
    }

    ~GraphWindow()
    {
        // optional: de-allocate all resources once they've outlived their purpose:
        // ------------------------------------------------------------------------
        glDeleteVertexArrays(1, &m_vertex_array);
        glDeleteBuffers(1, &m_vertex_buffer);
        glDeleteProgram(m_shader_program);

        glfwDestroyWindow(m_window);
    }

    void spin()
    {
        // render loop
        // -----------
        while (!glfwWindowShouldClose(m_window))
        {
            // input
            // -----
            process_input();

            // render
            // ------
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            // draw our first triangle
            glUseProgram(m_shader_program);

            // double xpos, ypos;
            // glfwGetCursorPos(m_window, &xpos, &ypos);

            // int width, height;
            // glfwGetWindowSize(m_window, &width, &height);

            glUniform1f(m_uniform_scale_x, m_zoom);

            glBindVertexArray(m_vertex_array); // seeing as we only have a single vertex_array there's no need to bind it every time, but we'll do so to keep things a bit more organized

            // auto value = std::sin(static_cast<double>(time) / 500.0);
            // ts.push(time++, value);

            // auto buf = ts.mean(0, 100, 2000);
            // for (int i = 0; i < 2000; i++)
            // {
            //     graph[i].x = i;
            //     graph[i].y = buf[i];
            // }

            // vertices[0] += 0.001f;

            glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(graph), graph);

            glDrawArrays(GL_LINE_STRIP, 0, 2000);
            // glBindVertexArray(0); // no need to unbind it every time

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(m_window);
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
        GraphWindow *obj = static_cast<GraphWindow *>(glfwGetWindowUserPointer(window));
        obj->m_cursor = glm::vec2(xpos, ypos);
    }

    static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
    {
        std::cout << "Scroll " << yoffset << '\n';

        GraphWindow *obj = static_cast<GraphWindow *>(glfwGetWindowUserPointer(window));
        obj->m_zoom *= 1.0 + (yoffset / 10);
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
        // vertex shader
        ShaderLoader vertex_loader("/home/steve/Development/glot/vertex.glsl");
        auto vertexShaderSourceStr = vertex_loader.load();
        const char *vertexShaderSource = vertexShaderSourceStr.c_str();
        int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        // check for shader compile errors
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                      << infoLog << std::endl;
        }

        // fragment shader
        ShaderLoader loader("/home/steve/Development/glot/fragment.glsl");
        auto fragmentShaderSourceStr = loader.load();
        const char *fragmentShaderSource = fragmentShaderSourceStr.c_str();
        int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        // check for shader compile errors
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                      << infoLog << std::endl;
        }

        // Geometry shader
        ShaderLoader geomLoader("/home/steve/Development/glot/geometry.glsl");
        auto geomShaderSourceStr = geomLoader.load();
        const char *geomShaderSource = geomShaderSourceStr.c_str();
        int geomShader = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(geomShader, 1, &geomShaderSource, NULL);
        glCompileShader(geomShader);
        // check for shader compile errors
        glGetShaderiv(geomShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cout << "ERROR::SHADER::GEOM::COMPILATION_FAILED\n"
                      << infoLog << std::endl;
        }

        // link shaders
        m_shader_program = glCreateProgram();
        glAttachShader(m_shader_program, vertexShader);
        glAttachShader(m_shader_program, geomShader);
        glAttachShader(m_shader_program, fragmentShader);
        glLinkProgram(m_shader_program);
        // check for linking errors
        glGetProgramiv(m_shader_program, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(m_shader_program, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                      << infoLog << std::endl;
            throw std::runtime_error("Error linking shader");
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteShader(geomShader);

        m_uniform_scale_x = glGetUniformLocation(m_shader_program, "scale_x");
        glUseProgram(m_shader_program);
        glUniform1f(m_uniform_scale_x, 1.0f);
    }

    static constexpr unsigned int SCR_WIDTH = 800;
    static constexpr unsigned int SCR_HEIGHT = 600;

    GLFWwindow *m_window;
    int m_shader_program, m_uniform_scale_x;
    unsigned int m_vertex_array, m_vertex_buffer;
    glm::vec2 m_cursor;
    double m_zoom;
    point graph[2000];
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

    // for (int i = 0; i < 2000; i++)
    // {
    //     float x = (i - 1000.0) / 100.0;
    //     graph[i].x = x;
    //     graph[i].y = std::sin(x * 10.0) / (1.0 + x * x);
    // }

    glfwTerminate();

    return 0;
}
