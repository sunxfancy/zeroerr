cmake_minimum_required(VERSION 3.15)

project(ZeroErr)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/third_party)

file(GLOB_RECURSE source_files ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_library(zeroerr STATIC ${source_files})


find_package(LLVM CONFIG)

if (LLVM_FOUND)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

add_definitions(-DHAVE_LLVM)

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs support core)

# Link against LLVM libraries
message(STATUS "Linking against LLVM libraries: ${llvm_libs}")
else()
set(llvm_libs "")
endif()


file(GLOB_RECURSE test_files ${CMAKE_SOURCE_DIR}/test/*.cpp)

add_executable(unittest ${test_files})
target_link_libraries(unittest zeroerr ${llvm_libs})

