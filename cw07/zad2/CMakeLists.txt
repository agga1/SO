cmake_minimum_required(VERSION 3.16)
project(zad2 C)

set(CMAKE_C_STANDARD 99)

add_executable(zad2 main.c common.h creator.c packer.c sender.c)