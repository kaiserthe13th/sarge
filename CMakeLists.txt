cmake_minimum_required(VERSION 3.29)
project(sarge C)

set(CMAKE_C_STANDARD 11)

add_library(sarge STATIC sarge.c)
add_executable(example example.c)

target_link_libraries(example sarge)
