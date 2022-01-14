cmake_minimum_required(VERSION 3.16)
project(glot)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(glot
	src/glot.cpp
	src/shader.cpp
	src/shader_program.cpp)

target_link_libraries(glot ${CONAN_LIBS})

# Copy the shaders over to the build dir
configure_file(shaders/vertex.glsl . COPYONLY)
configure_file(shaders/geometry.glsl . COPYONLY)
configure_file(shaders/fragment.glsl . COPYONLY)
