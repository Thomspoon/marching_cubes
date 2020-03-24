cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(advanced_shaders VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17 -g -O2")

add_subdirectory(external)
add_subdirectory(src)
