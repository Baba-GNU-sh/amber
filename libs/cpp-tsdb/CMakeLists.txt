cmake_minimum_required(VERSION 3.16)
project(tsdb)

option(BUILD_TESTS "Build test programs" OFF)
option(BUILD_COVERAGE "Build with coverage info" OFF)
option(TSDB_BUILD_EXAMPLES "Build example binaries" OFF)

add_library(tsdb INTERFACE)
target_include_directories(
	tsdb
	INTERFACE include
)

if (BUILD_TESTS)
	find_package(GTest REQUIRED)
	find_package(Boost REQUIRED COMPONENTS timer)

	enable_testing()
	add_executable(test_tsdb test/test.cpp)

	target_link_libraries(
		test_tsdb
		PRIVATE
			tsdb
			Boost::timer
			GTest::GTest
	)
	
	if (BUILD_COVERAGE)
		target_link_libraries(
			test_tsdb
			PRIVATE
				tsdb
				Boost::timer
				GTest::GTest
				gcov
		)

		target_compile_options(test_tsdb PRIVATE -fprofile-arcs -ftest-coverage)
	else()
		target_link_libraries(
			test_tsdb
			PRIVATE
				tsdb
				Boost::timer
				GTest::GTest
		)
	endif()


	add_test(NAME AllTests COMMAND test_tsdb)
endif()

if (TSDB_BUILD_EXAMPLES)
	add_executable(example example.cpp)
	target_link_libraries(
		example
		PRIVATE
			tsdb
			Boost::timer)
endif()
