cmake_minimum_required(VERSION 3.16)
project(glot)

option(BUILD_TESTS "Build test programs" OFF)

if (BUILD_TESTS)
	enable_testing()
endif()

add_subdirectory(libs/cpp-tsdb)

find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)

add_executable(glot
	src/glot.cpp
	src/bindings/imgui_impl_opengl3.cpp
	src/bindings/imgui_impl_glfw.cpp
	src/shader.cpp
	src/shader_program.cpp)

target_link_libraries(glot glfw::glfw glad::glad glm::glm imgui::imgui)

# Copy the shaders over to the build dir
configure_file(shaders/vertex.glsl . COPYONLY)
configure_file(shaders/geometry.glsl . COPYONLY)
configure_file(shaders/fragment.glsl . COPYONLY)

configure_file(shaders/simple/vertex.glsl simple_vertex.glsl COPYONLY)
configure_file(shaders/simple/fragment.glsl simple_fragment.glsl COPYONLY)

configure_file(shaders/char_vert.glsl char_vert.glsl COPYONLY)
configure_file(shaders/char_frag.glsl char_frag.glsl COPYONLY)

configure_file(font.png font.png COPYONLY)

add_subdirectory(src/fontgen)
