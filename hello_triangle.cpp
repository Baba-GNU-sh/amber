#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader_loader.hpp"
#include "tsdb/tsdb.hpp"

#include <random>
#include <iostream>
#include <cmath>

static double zoom = 0.01;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    std::cout << "Scroll" << yoffset << '\n';
    zoom *= 1.0 + (yoffset/10);
}


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

const char *vertexShaderSource = "#version 330 core\n"
                                 "attribute vec2 coord2d;\n"
                                 "uniform float offset_x;\n"
                                 "uniform float scale_x;\n"
                                 "void main()\n"
                                 "{\n"
                                 "   gl_Position = vec4((coord2d.x * scale_x) + offset_x, coord2d.y, 0, 1);\n"
                                 "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
                                   "out vec4 FragColor;\n"
                                   "void main()\n"
                                   "{\n"
                                   "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
                                   "}\n\0";

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
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
    int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, geomShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
    int vertexColorLocation = glGetUniformLocation(shaderProgram, "scale_x");
    int offsetLocation = glGetUniformLocation(shaderProgram, "offset_x");
    glUseProgram(shaderProgram);
    glUniform1f(vertexColorLocation, .5f);

    struct point
    {
        GLfloat x;
        GLfloat y;
    };

    point graph[2000];
    point graph_wibbles[2000];

    std::random_device dev;
    std::mt19937 rng(dev());

    std::uniform_real_distribution<> dist(-.1, .1);

    // std::cout <<  << std::endl;

    for (int i = 0; i < 2000; i++)
    {
        float x = (i - 1000.0) / 100.0;
        graph[i].x = x;
        graph[i].y = std::sin(x * 10.0) / (1.0 + x * x);
    }

    unsigned int vertex_array;
    glGenVertexArrays(1, &vertex_array);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(vertex_array);

    unsigned int vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
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
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    SparseTimeSeries ts(true);
    Time time = 0;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw our first triangle
        glUseProgram(shaderProgram);

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        int width, height;
        glfwGetWindowSize(window, &width, &height);

        glUniform1f(vertexColorLocation, zoom);
        glUniform1f(offsetLocation, -1);

        glBindVertexArray(vertex_array); // seeing as we only have a single vertex_array there's no need to bind it every time, but we'll do so to keep things a bit more organized

        auto value = std::sin(static_cast<double>(time) / 100.0);
        ts.push(time++, value);

        auto buf = ts.mean(0, 1, 2000);
        for (int i = 0; i < 2000; i++)
        {
            graph[i].x = i;
            graph[i].y = buf[i];
        }

        // std::cout << buf.size() << '\n';

        // vertices[0] += 0.001f;

        // for (int i = 0; i < 2000; i++)
        // {
        //     graph_wibbles[i].x = graph[i].x;
        //     graph_wibbles[i].y = graph[i].y + dist(rng);
        // }

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(graph), graph);

        glDrawArrays(GL_LINE_STRIP, 0, 2000);
        // glBindVertexArray(0); // no need to unbind it every time

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &vertex_array);
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteProgram(shaderProgram);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

static void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    // std::cout << xpos << '\n';
}
