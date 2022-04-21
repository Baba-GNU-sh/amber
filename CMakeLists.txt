cmake_minimum_required(VERSION 3.16)
project(glot)

option(BUILD_TESTS "Build test programs" OFF)
option(BUILD_BENCHMARKS "Build benchmark programs" OFF)
option(BUILD_COVERAGE "Build with coverage instrumentation" OFF)
option(USE_SANITIZERS "Build with address and undefined behaviour sanitizer instrumentation" OFF)

set(CMAKE_CXX_STANDARD 17)

# Note: We need to enable testing at the top level to generate the test file at the root of the build dir
if (BUILD_TESTS)
	enable_testing()
endif()

add_subdirectory(src/glot)

add_executable(main src/glot/main.cpp)
target_link_libraries(main PRIVATE glot)
if (NOT MSVC)
	target_compile_options(main PRIVATE -Werror -Wall -Wextra)
endif()
set_target_properties(main PROPERTIES OUTPUT_NAME glot)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)

# Copy the shaders over to the build dir
configure_file(shaders/plot/vertex.glsl plot/vertex.glsl COPYONLY)
configure_file(shaders/plot/geometry.glsl plot/geometry.glsl COPYONLY)
configure_file(shaders/plot/fragment.glsl plot/fragment.glsl COPYONLY)

configure_file(shaders/line/vertex.glsl line/vertex.glsl COPYONLY)
configure_file(shaders/line/fragment.glsl line/fragment.glsl COPYONLY)

configure_file(shaders/glyph/vertex.glsl glyph/vertex.glsl COPYONLY)
configure_file(shaders/glyph/fragment.glsl glyph/fragment.glsl COPYONLY)
configure_file(fonts/proggy_clean.png fonts/proggy_clean.png COPYONLY)

configure_file(audio/Lurking_Threat_3.wav audio/Lurking_Threat_3.wav COPYONLY)
configure_file(assets/marker_center.png assets/marker_center.png COPYONLY)
