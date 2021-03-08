cmake_minimum_required(VERSION 3.16)
project(glot)

add_executable(glot hello_triangle.cpp glad/src/glad.c)
target_include_directories(glot PRIVATE glad/include)
target_link_libraries(glot GL GLU glfw dl)
