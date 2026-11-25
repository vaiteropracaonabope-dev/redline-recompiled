cmake_minimum_required(VERSION 3.22.1)
project(redline_android LANGUAGES C CXX)

add_library(redline_android SHARED android_runtime.cpp)

find_library(log-lib log)
target_link_libraries(redline_android PRIVATE ${log-lib})
target_compile_features(redline_android PRIVATE cxx_std_17)
