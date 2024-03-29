cmake_minimum_required(VERSION 3.16)
project(amber)

set(CMAKE_CXX_STANDARD 17)

# Note: We need to enable testing at the top level to generate the test file at the root of the build dir
if (BUILD_TESTS)
	enable_testing()
endif()

add_subdirectory(src/amber)

add_executable(main src/amber/main.cpp)
target_link_libraries(main PRIVATE amber)
if (NOT MSVC)
	target_compile_options(main PRIVATE -Werror -Wall -Wextra)
endif()
set_target_properties(main PROPERTIES OUTPUT_NAME amber)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT main)

add_executable(demo src/amber/demo.cpp)
target_link_libraries(demo PRIVATE amber)

# Copy assets and shaders to the build dir
configure_file(shaders/plot/vertex.glsl shaders/plot/vertex.glsl COPYONLY)
configure_file(shaders/plot/geometry.glsl shaders/plot/geometry.glsl COPYONLY)
configure_file(shaders/plot/fragment.glsl shaders/plot/fragment.glsl COPYONLY)

configure_file(shaders/block/vertex.glsl shaders/block/vertex.glsl COPYONLY)
configure_file(shaders/block/fragment.glsl shaders/block/fragment.glsl COPYONLY)

configure_file(shaders/sprite/vertex.glsl shaders/sprite/vertex.glsl COPYONLY)
configure_file(shaders/sprite/fragment.glsl shaders/sprite/fragment.glsl COPYONLY)

configure_file(assets/fonts/proggy_clean.png assets/fonts/proggy_clean.png COPYONLY)
configure_file(assets/sprites/marker_center.png assets/sprites/marker_center.png COPYONLY)
configure_file(assets/sprites/marker_left.png assets/sprites/marker_left.png COPYONLY)
configure_file(assets/sprites/marker_right.png assets/sprites/marker_right.png COPYONLY)

configure_file(audio/Lurking_Threat_3.wav audio/Lurking_Threat_3.wav COPYONLY)
