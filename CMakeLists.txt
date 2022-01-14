cmake_minimum_required(VERSION 3.16)
project(glot)

enable_testing()

add_subdirectory(libs/cpp-tsdb)

find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)

add_executable(glot
	src/glot.cpp
	src/shader.cpp
	src/shader_program.cpp)

target_link_libraries(glot glfw::glfw glad::glad glm::glm)

# Copy the shaders over to the build dir
configure_file(shaders/vertex.glsl . COPYONLY)
configure_file(shaders/geometry.glsl . COPYONLY)
configure_file(shaders/fragment.glsl . COPYONLY)
