cmake_minimum_required(VERSION 3.16)

find_package(Freetype REQUIRED)

add_executable(
    fontgen
    main.cpp
    fontgen.cpp
)

target_link_libraries(
    fontgen
    Freetype::Freetype
)
