cmake_minimum_required(VERSION 3.31)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IMPL "cpp" CACHE STRING "Implementation to build: cpp or zig")
set_property(CACHE IMPL PROPERTY STRINGS cpp zig)
if(NOT IMPL STREQUAL "cpp" AND NOT IMPL STREQUAL "zig")
    message(FATAL_ERROR "Unknown IMPL='${IMPL}'. Use 'cpp' or 'zig'.")
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

project(band-hero)
