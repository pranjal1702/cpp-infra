add_library(mpmc_mutex_queue INTERFACE)

target_include_directories(
    mpmc_mutex_queue
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)