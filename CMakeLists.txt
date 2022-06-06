cmake_minimum_required(VERSION 3.15)

project(ZeroErr)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB_RECURSE source_files ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_library(zeroerr STATIC ${source_files})

file(GLOB_RECURSE test_files ${CMAKE_SOURCE_DIR}/test/*.cpp)

add_executable(unittest ${test_files})
target_link_libraries(unittest zeroerr)

