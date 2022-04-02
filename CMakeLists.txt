cmake_minimum_required(VERSION 3.16)
project(glot)

set(CMAKE_CXX_STANDARD 17)

option(BUILD_TESTS "Build test programs" OFF)

if (BUILD_TESTS)
	enable_testing()
endif()

add_subdirectory(libs/cpp-tsdb)
add_subdirectory(src/glot)

# Copy the shaders over to the build dir
configure_file(shaders/plot/vertex.glsl plot/vertex.glsl COPYONLY)
configure_file(shaders/plot/geometry.glsl plot/geometry.glsl COPYONLY)
configure_file(shaders/plot/fragment.glsl plot/fragment.glsl COPYONLY)

configure_file(shaders/line/vertex.glsl line/vertex.glsl COPYONLY)
configure_file(shaders/line/fragment.glsl line/fragment.glsl COPYONLY)

configure_file(shaders/glyph/vertex.glsl glyph/vertex.glsl COPYONLY)
configure_file(shaders/glyph/fragment.glsl glyph/fragment.glsl COPYONLY)
configure_file(fonts/proggy_clean.png fonts/proggy_clean.png COPYONLY)

configure_file(audio/CantinaBand60.wav audio/CantinaBand60.wav COPYONLY)
