cmake_minimum_required(VERSION 3.16)
project(tsdb)

option(BUILD_TESTS "Build test programs" OFF)
option(TSDB_BUILD_EXAMPLES "Buld example binaries" OFF)

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
