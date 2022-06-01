find_package(glfw3 REQUIRED)
find_package(glad REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(spdlog REQUIRED)
find_package(audiofile REQUIRED)
find_package(Boost REQUIRED)

add_subdirectory(bindings)
add_subdirectory(stb_image)
add_subdirectory(database)

set(CMAKE_CXX_STANDARD 17)

add_library(
	glot
	STATIC
		shader.cpp
		shader_program.cpp
		plugin_context.cpp
		plugin_manager.cpp
		window_glfw.cpp
		window_glfw_imgui.cpp
		audiofile_plugin.cpp
		wavegen_plugin.cpp
		graph.cpp
		label.cpp
		plot.cpp
		sprite.cpp
		font.cpp
		marker.cpp
		selection_box.cpp
		axis.cpp
		view.cpp
		ui.cpp
)
target_include_directories(glot PUBLIC .)
target_link_libraries(
	glot
	PUBLIC
		bindings
		stb_image
		database
		glfw::glfw
		glad::glad
		glm::glm
		imgui::imgui
		spdlog::spdlog
		audiofile::audiofile
		Boost::boost
)

if (USE_IWYU)
	find_program(iwyu_path NAMES include-what-you-use iwyu REQUIRED)
	set_property(TARGET glot PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
endif()

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

if (BUILD_TESTS)
	find_package(GTest REQUIRED)
	add_executable(tests
		test/test_transform.cpp
		test/test_graph_utils.cpp
	)
	target_link_libraries(
		tests
		PRIVATE
			glot
			GTest::GTest
	)
	if (BUILD_COVERAGE)
		target_compile_options(tests PRIVATE -fprofile-arcs -ftest-coverage)
		target_link_options(tests PUBLIC -fprofile-arcs -ftest-coverage)
	endif()
	add_test(NAME Tests COMMAND tests)
endif()
