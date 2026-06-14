add_library(thread_pool INTERFACE)

target_include_directories(
    thread_pool
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
    thread_pool
    INTERFACE
    mpmc_mutex_queue
)