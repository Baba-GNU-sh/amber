cmake_minimum_required(VERSION 3.16)
project(glot)

find_package(OpenGL REQUIRED)

# Build GLFW from source - it's easier than trying to install it on all platforms - especially windows
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libs/glfw-3.3.6)

# Add tsdb
set(TSDB_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(cpp-tsdb)

# Build the glot executable
add_executable(glot hello_triangle.cpp libs/glad/src/glad.c)

target_include_directories(glot
	PRIVATE
	libs/glad/include)

#target_link_libraries(glot OpenGL::GL GLU glfw dl)
target_link_libraries(glot OpenGL::GL glfw tsdb)
