cmake_minimum_required(VERSION 3.22.1)
project(redline_android LANGUAGES C CXX)

set(REDLINE_RUNTIME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../../runtime")

add_library(redline_android SHARED
    android_runtime.cpp
    ${REDLINE_RUNTIME_DIR}/host_fs_posix.c
    ${REDLINE_RUNTIME_DIR}/host_sync_posix.c
    ${REDLINE_RUNTIME_DIR}/host_thread_posix.c
)

target_include_directories(redline_android PRIVATE "${REDLINE_RUNTIME_DIR}")
target_compile_features(redline_android PRIVATE c_std_11 cxx_std_17)
target_compile_options(redline_android PRIVATE -Wall -Wextra)

find_library(log-lib log)
target_link_libraries(redline_android PRIVATE ${log-lib})
