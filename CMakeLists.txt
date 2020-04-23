cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(advanced_shaders VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

IF(WIN32)
IF(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /ZI")
ELSE()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -std=c++17 -O2 -g")
ENDIF()
ELSE()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -std=c++17 -O2 -g")
ENDIF()

add_subdirectory(external)
add_subdirectory(src)
