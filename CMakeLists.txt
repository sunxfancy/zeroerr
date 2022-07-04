cmake_minimum_required(VERSION 3.15)

project(ZeroErr LANGUAGES CXX)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

option(COLORFUL_OUTPUT "Use colorful output(ON, OFF, AUTO)" "AUTO")

if(COLORFUL_OUTPUT EQUAL "ON")
    add_definitions(-DZEROERR_ALWAYS_COLORFUL)
endif()

if(COLORFUL_OUTPUT EQUAL "OFF")
    add_definitions(-DZEROERR_DISABLE_COLORFUL)
endif()

option(ENABLE_THREAD_SAFE "Enable thread-safe(ON, OFF)" ON)

if(NOT ENABLE_THREAD_SAFE)
    add_definitions(-DZEROERR_NO_THREAD_SAFE)
endif()

option(ENABLE_AUTO_INIT "Enable auto-init(ON, OFF)" ON)

if(NOT ENABLE_AUTO_INIT)
    add_definitions(-DZEROERR_DISABLE_AUTO_INIT)
endif()

option(USE_MOLD "Use mold for linking" ON)

if(USE_MOLD)
    find_program(HAS_MOLD "mold")

    if(HAS_MOLD)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=mold")
        endif()

        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 12.1)
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=mold")
            else()
            endif()
        endif()
    endif()
endif()

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
include_directories(${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/third_party)

file(GLOB_RECURSE source_files ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_library(zeroerr STATIC ${source_files})

# if(ENABLE_SAN)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize-recover=all -fsanitize=address -fsanitize=undefined -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize-recover=address -fsanitize=address")

# endif()
find_package(LLVM CONFIG)

if(LLVM_FOUND)
    if(CMAKE_CXX_STANDARD GREATER_EQUAL 14)
        message(STATUS " Found LLVM ${LLVM_PACKAGE_VERSION} ")
        message(STATUS " Using LLVMConfig.cmake in: ${LLVM_DIR} ")
        include_directories(${LLVM_INCLUDE_DIRS})
        link_directories(${LLVM_LIBRARY_DIRS})

        add_definitions(-DHAVE_LLVM)

        # Find the libraries that correspond to the LLVM components
        # that we wish to use
        llvm_map_components_to_libnames(llvm_libs support core)

        # Link against LLVM libraries
        message(STATUS " Linking against LLVM libraries: ${llvm_libs} ")
    endif()
else()
    set(llvm_libs " ")
endif()

include(CheckLanguage)
check_language(CUDA)

if(CMAKE_CUDA_COMPILER)
    enable_language(CUDA)
    message(STATUS " CUDA ${CMAKE_CUDA_COMPILER_ID} ${CMAKE_CUDA_COMPILER_VERSION} enabled ")

    set(CMAKE_CUDA_STANDARD 17)
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)

    set(CMAKE_CUDA_ARCHITECTURES 80)
    set(CMAKE_CUDA_FLAGS " -ccbin=/usr/bin/gcc-10 ")

    file(GLOB_RECURSE cuda_files ${CMAKE_SOURCE_DIR}/test/*.cu)
else()
    set(cuda_files " ")
endif()

file(GLOB_RECURSE test_files ${CMAKE_SOURCE_DIR}/test/*.cpp)

add_executable(unittest ${test_files} ${cuda_files})
target_link_libraries(unittest zeroerr ${llvm_libs})

enable_testing()
add_test(NAME RunUnitTest COMMAND unittest)