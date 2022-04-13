find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(spdlog REQUIRED)
find_package(audiofile REQUIRED)
find_package(Boost REQUIRED)

add_subdirectory(bindings)

set(CMAKE_CXX_STANDARD 17)

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
		window.cpp
		audiofile_plugin.cpp
		wavegen_plugin.cpp
		random_data_plugin.cpp
		imgui_window.cpp
		app_context.cpp
		timeseries_dense.cpp
		timeseries_sparse.cpp
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
		Boost::boost
)

if (NOT MSVC)
	target_compile_options(glot PRIVATE -Werror -Wall -Wextra)
endif()

if (USE_SANITIZERS)
	target_compile_options(glot PRIVATE -fsanitize=address -fsanitize=undefined)
	target_link_options(glot PUBLIC -fsanitize=address -fsanitize=undefined)
endif()

if (BUILD_COVERAGE)
	target_compile_options(glot PRIVATE -fprofile-arcs -ftest-coverage)
	target_link_options(glot PUBLIC -fprofile-arcs -ftest-coverage)
endif()

if (BUILD_BENCHMARKS)
	find_package(benchmark REQUIRED)
	add_executable(bench benchmark/bench_timeseries_dense.cpp)
	target_link_libraries(
		bench
		PRIVATE
			glot
			benchmark::benchmark
	)
	add_test(NAME Benchmarks COMMAND bench)
endif()

if (BUILD_TESTS)
	find_package(GTest REQUIRED)
	add_executable(tests
		test/test_timeseries_dense.cpp
		test/test_timeseries_sparse.cpp
		test/test_chunked_vector.cpp
	)
	target_link_libraries(
		tests
		PRIVATE
			glot
			GTest::GTest
	)
	add_test(NAME Tests COMMAND tests)
endif()
