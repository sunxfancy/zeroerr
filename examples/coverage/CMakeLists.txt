cmake_minimum_required(VERSION 3.15)
project(ZeroErrCoverageExample LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
option(ZEROERR_ENABLE_COVERAGE "Enable the coverage example" ON)
add_subdirectory(../.. zeroerr)

add_library(calculator STATIC calculator.cpp)
add_executable(coverage_example tests.cpp)
target_link_libraries(coverage_example PRIVATE calculator zeroerr)

if(ZEROERR_ENABLE_COVERAGE)
    zeroerr_instrument_coverage(calculator)
    zeroerr_add_coverage(coverage_example)
endif()
