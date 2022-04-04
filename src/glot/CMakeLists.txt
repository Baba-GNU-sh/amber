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

add_library(
	glot
	STATIC
		shader.cpp
		shader_program.cpp
		graph.cpp
		plot.cpp
		database.cpp
		plugin_context.cpp
		plugin_manager.cpp
		timeseries_dense.cpp
		window.cpp
		audiofile_plugin.cpp
		wavegen_plugin.cpp
)
target_include_directories(glot PUBLIC .)
target_link_libraries(
	glot
	PUBLIC
		bindings
		glfw::glfw
		glad::glad
		glm::glm
		imgui::imgui
		spdlog::spdlog
		audiofile::audiofile
)
target_compile_options(glot PRIVATE -Werror -Wall -Wextra)

add_executable(glotapp main.cpp)
target_link_libraries(glotapp PRIVATE glot)
target_compile_options(glotapp PRIVATE -Werror -Wall -Wextra)

if (BUILD_BENCHMARKS)
	find_package(benchmark REQUIRED)
	add_executable(bench benchmark/ts.cpp)
	target_link_libraries(
		bench
		PRIVATE
			glot
			benchmark::benchmark
	)
	add_test(NAME Benchmarks COMMAND bench)
endif()
