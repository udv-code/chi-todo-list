cmake_minimum_required(VERSION 3.17)
project(todo_list)

set(CMAKE_CXX_STANDARD 17)

include_directories($ENV{UniversalCRTSdkDir}/Include/ucrt)
include_directories($ENV{UniversalCRTSdkDir}/Include/10.0.10056.0/ucrt)

add_subdirectory(list-main)
add_subdirectory(examples)
