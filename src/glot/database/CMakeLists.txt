add_library(
	database
	STATIC
		src/database.cpp
		src/timeseries_dense.cpp
)

target_include_directories(
	database
	PRIVATE
		include/database
	PUBLIC
		include
)

if (USE_SANITIZERS)
	target_compile_options(database PRIVATE -fsanitize=address -fsanitize=undefined)
	target_link_options(database PUBLIC -fsanitize=address -fsanitize=undefined)
endif()

if (USE_COVERAGE)
	target_compile_options(database PRIVATE -fprofile-arcs -ftest-coverage)
	target_link_options(database PUBLIC -fprofile-arcs -ftest-coverage)
endif()

if (BUILD_BENCHMARKS)
	find_package(benchmark REQUIRED)
	add_executable(database_bench benchmark/bench_timeseries_dense.cpp)
	target_link_libraries(
		database_bench
		PRIVATE
			database
			benchmark::benchmark
	)
	add_test(NAME Benchmarks COMMAND database_bench)
endif()

if (BUILD_TESTS)
	find_package(GTest REQUIRED)
	add_executable(database_tests
		test/test_timeseries_dense.cpp
		test/test_chunked_vector.cpp
		test/test_database.cpp
	)
	target_link_libraries(
		database_tests
		PRIVATE
			glot
			GTest::GTest
	)
	if (USE_COVERAGE)
		target_compile_options(database_tests PRIVATE -fprofile-arcs -ftest-coverage)
		target_link_options(database_tests PUBLIC -fprofile-arcs -ftest-coverage)
	endif()
	add_test(NAME Tests COMMAND database_tests)
endif()
