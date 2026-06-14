cmake_minimum_required(VERSION 3.20)

project(cpp_infra LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(libs/mpmc_mutex_queue)
add_subdirectory(libs/thread_pool)

add_executable(example
    examples/main.cpp
)

target_link_libraries(
    example
    PRIVATE
    thread_pool
)