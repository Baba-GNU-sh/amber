cmake_minimum_required(VERSION 3.16)
project(glot)

find_package(OpenGL REQUIRED)

# Build GLFW from source - easier than trying to install the binary package on Windows!
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/glfw-3.3.6)

# Include GLM from source - again ... easier on windows..!
add_subdirectory(libs/glm-0.9.9.8)

# Add tsdb
set(TSDB_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/tsdb)

# Build the glot executable
add_executable(
	glot
	src/glot.cpp
	src/shader.cpp
	src/shader_program.cpp
	libs/glad/src/glad.c)

target_include_directories(glot
	PRIVATE
	libs/glad/include)

file(COPY shaders DESTINATION .)

configure_file(shaders/vertex.glsl shaders/ COPYONLY)
configure_file(shaders/geometry.glsl shaders/ COPYONLY)
configure_file(shaders/fragment.glsl shaders/ COPYONLY)

#target_link_libraries(glot OpenGL::GL GLU glfw dl)
target_link_libraries(glot OpenGL::GL glm::glm glfw tsdb)
