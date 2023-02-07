cmake_minimum_required(VERSION 3.15)

project(transport VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment)
add_link_options(-fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC")

add_subdirectory(core)
add_executable(main main.cpp)

target_link_libraries(main PUBLIC core)
target_include_directories(main PUBLIC
        "${PROJECT_SOURCE_DIR}/profiler"
        "${PROJECT_SOURCE_DIR}/utestlib")