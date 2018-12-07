cmake_minimum_required(VERSION 3.12)
project(aocd4 C)

set(CMAKE_C_STANDARD 99)

add_executable(aocd4 main.c)

configure_file(input.in input.in COPYONLY)