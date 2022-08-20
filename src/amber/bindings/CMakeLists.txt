find_package(imgui REQUIRED)
find_package(glfw3 REQUIRED)

add_library(
    bindings
    STATIC
	imgui_impl_opengl3.cpp
	imgui_impl_glfw.cpp
)

target_link_libraries(
	bindings
	imgui::imgui
    glfw::glfw
)
