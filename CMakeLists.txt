cmake_minimum_required(VERSION 3.15)

project(transport VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(core)
add_executable(main main.cpp)

target_link_libraries(main PUBLIC core)
target_include_directories(main PUBLIC
        "${PROJECT_SOURCE_DIR}/profiler"
        "${PROJECT_SOURCE_DIR}/utestlib")
