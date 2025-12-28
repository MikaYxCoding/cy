cmake_minimum_required(VERSION 3.31)

project(CY VERSION 3.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

option(CY_BUILD_EXTRA "Build CY test/example code" OFF)

if ("${CMAKE_CURRENT_SOURCE_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}")
    message("-- Building CY as root project")
    set(CY_BUILD_EXTRA ON)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

add_library(cy INTERFACE)
target_include_directories(cy INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include/")

if (CY_BUILD_EXTRA)
add_executable(result example/Result.cpp)
target_link_libraries(result PRIVATE cy)
endif()