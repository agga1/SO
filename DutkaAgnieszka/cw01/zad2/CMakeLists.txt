cmake_minimum_required(VERSION 3.5)
project(lab1 C)

set(CMAKE_C_STANDARD 11)
add_executable(main main.c)

add_library(lab1 library.c library.h)
target_link_libraries(main lab1)
