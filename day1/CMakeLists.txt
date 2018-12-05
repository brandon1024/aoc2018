cmake_minimum_required(VERSION 3.12)
project(aocd1 C)

set(CMAKE_C_STANDARD 99)

add_executable(aocd1 main.c)

configure_file(input.in input.in COPYONLY)