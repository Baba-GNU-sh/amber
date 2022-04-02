option(BUILD_TESTS "Build test programs" OFF)

if (BUILD_TESTS)
	enable_testing()
endif()

add_subdirectory(bindings)

find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(spdlog REQUIRED)
find_package(audiofile REQUIRED)

add_executable(glot
	shader.cpp
	shader_program.cpp
	graph.cpp
	plot.cpp
	main.cpp
	audiofile_plugin.cpp
	database.cpp
	plugin_context.cpp
	plugin_manager.cpp
	timeseries_dense.cpp
	window.cpp
)

target_link_libraries(
	glot
	bindings
	glfw::glfw
	glad::glad
	glm::glm
	imgui::imgui
	spdlog::spdlog
	audiofile::audiofile
)

target_compile_options(glot PRIVATE -Werror -Wall -Wextra)
