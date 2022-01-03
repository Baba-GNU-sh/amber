cmake_minimum_required(VERSION 3.16)
project(tsdb)

if(TSDB_BUILD_TESTS)
	find_package(GTest REQUIRED)
	find_package(Boost REQUIRED COMPONENTS timer)
endif()

add_library(tsdb INTERFACE)
target_include_directories(
	tsdb
	INTERFACE include
)

if(TSDB_BUILD_TESTS)
	add_executable(example example.cpp)
	target_link_libraries(
		example
		PRIVATE
			tsdb
			Boost::timer)

	enable_testing()
	add_executable(test_tsdb test.cpp)
	target_link_libraries(
		test_tsdb
		PRIVATE
			tsdb
			Boost::timer
			GTest::GTest
			GTest::Main)
	add_test(AllTests test_tsdb)
endif()
