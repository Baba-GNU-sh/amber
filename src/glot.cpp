#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <glm/fwd.hpp>
#include <glm/matrix.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <random>

#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"
#include <imgui.h>

#include "shader_utils.hpp"

#include <cmath>
#include <iostream>
#include <random>
#include <stdexcept>

#include "graph.hpp"

class Window
{
  public:
	Window()
	{
		m_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "GLot", NULL, NULL);
		if (m_window == NULL) {
			glfwTerminate();
			throw std::runtime_error("Failed to create GLFW window");
		}

		glfwSetWindowUserPointer(m_window, this);
		glfwMakeContextCurrent(m_window);
		glfwSetFramebufferSizeCallback(m_window,
		                               Window::framebuffer_size_callback);
		glfwSetCursorPosCallback(m_window, Window::cursor_pos_callback);
		glfwSetScrollCallback(m_window, Window::scroll_callback);
		glfwSetMouseButtonCallback(m_window, Window::mouse_button_callback);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			throw std::runtime_error("Failed to initialize GLAD");
		}

		glEnable(GL_MULTISAMPLE);
		//glEnable(GL_LINE_SMOOTH);
		glLineWidth(2.0f);

		// Depths test helps us with the rendering for a small perf penalty
		glEnable(GL_DEPTH_TEST);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Disable vsync
		glfwSwapInterval(0);

		// Set the colour to be a nice dark green
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

		// Initialize the scene
		init_imgui();

		m_graph = std::make_shared<GraphView>();
		m_graph->set_size(SCR_WIDTH, SCR_HEIGHT);

		update_viewport_matrix(SCR_WIDTH, SCR_HEIGHT);
	}

	~Window()
	{
		glfwDestroyWindow(m_window);
	}

	void update_viewport_matrix(int width, int height)
	{
		auto vp_matrix = glm::scale(glm::mat3x3(1.0f), glm::vec2(width / 2, -height / 2));
		vp_matrix = glm::translate(vp_matrix, glm::vec2(1, -1));

		m_graph->update_viewport_matrix(vp_matrix);
	}

	void init_imgui()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		(void)io;
		// io.ImGuiConfigFlags_WantCaptureMouse |=
		// ImGuiConfigFlags_WantCaptureMouse;
		// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable
		// Keyboard Controls io.ConfigFlags |=
		// ImGuiConfigFlags_NavEnableGamepad;
		// // Enable Gamepad Controls

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(m_window, true);

		const char* glsl_version = "#version 130";
		ImGui_ImplOpenGL3_Init(glsl_version);
	}

	void draw()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_graph->draw();

		render_imgui();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(m_window);
	}

	void render_imgui()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Debug Info");
		ImGui::Text("Application average %.1f ms/frame (%.1f FPS)",
		            1000.0f / ImGui::GetIO().Framerate,
		            ImGui::GetIO().Framerate);

		auto viewmat = m_graph->get_view_matrix();
		ImGui::Text("ViewMat:");
		for (int i = 0; i < 3; i++) {
			ImGui::Text(
			  "%f %f %f", viewmat[0][i], viewmat[1][i], viewmat[2][i]);
		}

		auto cursor_gs = m_graph->get_cursor_graphspace();
		ImGui::Text("Cursor: %f %f", cursor_gs.x, cursor_gs.y);

		if (ImGui::Checkbox("Enable VSync", &_enable_vsync))
		{
			if (_enable_vsync)
				glfwSwapInterval( 1 );
			else
				glfwSwapInterval(0);
		}

		if (ImGui::Checkbox("Multisampling", &_enable_multisampling))
		{
			if (_enable_multisampling)
				glEnable(GL_MULTISAMPLE);
			else {
				glDisable(GL_MULTISAMPLE);
			}
		}

		ImGui::SliderInt("Plot Thickness", m_graph->get_plot_thickness(), 1, 8);

		ImGui::End();

		ImGui::Render();
	}

	void spin()
	{
		while (!glfwWindowShouldClose(m_window)) {
			process_input();
			draw();
			glfwPollEvents();
		}
	}

  private:
	static void framebuffer_size_callback(GLFWwindow* window,
	                                      int width,
	                                      int height)
	{
		std::cout << "Window resized: " << width << "x" << height << "px\n";
		glViewport(0, 0, width, height);
		Window* obj = static_cast<Window*>(glfwGetWindowUserPointer(window));

		// Update the graph to fill the screen
		obj->m_graph->set_size(width, height);
		obj->update_viewport_matrix(width, height);
	}

	static void cursor_pos_callback(GLFWwindow* window,
	                                double xpos,
	                                double ypos)
	{
		Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
		win->m_graph->cursor_move(xpos, ypos);
	}

	static void scroll_callback(GLFWwindow* window,
	                            double xoffset,
	                            double yoffset)
	{
		Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
		win->m_graph->mouse_scroll(xoffset, yoffset);
	}

	static void mouse_button_callback(GLFWwindow* window,
	                                  int button,
	                                  int action,
	                                  int mods)
	{
		ImGuiIO& io = ImGui::GetIO();
		bool capture = io.WantCaptureMouse;
		if (capture)
			return;

		Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
		win->m_graph->mouse_button(button, action, mods);
	}

	void process_input()
	{
		if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(m_window, true);
		}
	}

	static constexpr unsigned int SCR_WIDTH = 800;
	static constexpr unsigned int SCR_HEIGHT = 600;

	GLFWwindow* m_window;
	std::shared_ptr<GraphView> m_graph;

	bool _enable_vsync = false;
	bool _enable_multisampling = true;
	int _plot_thickness = 1;
};

void
error_callback(int error, const char* msg)
{
	std::string s;
	s = " [" + std::to_string(error) + "] " + msg + '\n';
	std::cerr << s << std::endl;
}

int
main()
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
		Window win;
		std::cout << "Use the scroll wheel to zoom in and out, and use the "
		             "left mouse button to drag the canvas around\n";
		win.spin();
	}

	glfwTerminate();

	std::cout << "Bye!\n";

	return 0;
}
