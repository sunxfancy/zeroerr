cmake_minimum_required(VERSION 3.15)

project(ZeroErr LANGUAGES C CXX)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
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

option(DISABLE_CUDA_BUILD "Disable CUDA build(ON, OFF)" ON)

option(USE_MOLD "Use mold for linking" OFF)

if(USE_MOLD)
    find_program(HAS_MOLD "mold")
    message(STATUS "Find Mold: ${HAS_MOLD}")
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

option(BUILD_EXAMPLES "Build examples(ON, OFF)" OFF)
option(BUILD_DOC "Build documentation" OFF)



add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>" )
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>" )

# add_compile_options("$<$<CXX_COMPILER_ID:GNU>:-ftime-report>")

set(header_dirs ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/extension ${CMAKE_CURRENT_SOURCE_DIR}/third_party ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${header_dirs})

file(GLOB_RECURSE include_files ${CMAKE_CURRENT_SOURCE_DIR}/include/**/*.h)
file(GLOB_RECURSE source_files ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(zeroerr STATIC ${source_files})

target_compile_options(zeroerr PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
    )
# if(ENABLE_SAN)
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize-recover=all -fsanitize=address -fsanitize=undefined -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize-recover=address -fsanitize=address")
# endif()

# Determine if this project is built as a subproject (using add_subdirectory) or if it is the main project.
set(MAIN_PROJECT OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(MAIN_PROJECT ON)
endif()

if(MAIN_PROJECT)
    # Build examples when this project is built by itself
    if (BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()

    ## LLVM Support

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
            llvm_map_components_to_libnames(llvm_libs support)

            set(llvm_libs ${llvm_libs} LLVM)

            # Link against LLVM libraries
            message(STATUS " Linking against LLVM libraries: ${llvm_libs} ")
        endif()
    else()
        set(llvm_libs "")
    endif()

    ## CUDA Support

    include(CheckLanguage)
    check_language(CUDA)
    if(CMAKE_CUDA_COMPILER AND NOT DISABLE_CUDA_BUILD)
        enable_language(CUDA)
        message(STATUS " CUDA ${CMAKE_CUDA_COMPILER_ID} ${CMAKE_CUDA_COMPILER_VERSION} enabled ")

        set(CMAKE_CUDA_STANDARD 17)

        set(CMAKE_CUDA_ARCHITECTURES 80)

        file(GLOB_RECURSE cuda_files ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cu)
    else()
        set(cuda_files "")
    endif()

    ## Test Cases

    file(GLOB_RECURSE test_files ${CMAKE_CURRENT_SOURCE_DIR}/test/*.cpp)
    list(REMOVE_ITEM test_files ${CMAKE_CURRENT_SOURCE_DIR}/test/single_file_test.cpp)

    add_executable(unittest ${test_files} ${cuda_files})
    target_link_libraries(unittest zeroerr ${llvm_libs})
    target_compile_options(unittest PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
        )

    add_executable(single_file_test ${CMAKE_CURRENT_SOURCE_DIR}/test/single_file_test.cpp)
    add_dependencies(single_file_test assemble_single_header)

    enable_testing()
    add_test(NAME RunUnitTest COMMAND unittest)

    ## Single Header
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/zeroerr.hpp
        DEPENDS
            ${include_files}
            ${source_files}
            ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen-single-file.cmake
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen-single-file.cmake
        COMMENT "assembling the single header")

    add_custom_target(assemble_single_header ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/zeroerr.hpp)

    ## Documentation

    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        # request to configure the file
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.en.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.en @ONLY)
        message("Doxygen build started")

        # note the option ALL which allows to build the docs together with the application
        add_custom_target( doc_doxygen 
            COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/doc_doxygen/zh
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM )
        
        add_custom_target( doc_doxygen_en 
            COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/doc_doxygen/en
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.en
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM )

        add_custom_target( doxybook2  
            COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/docs/zh
            COMMAND ${CMAKE_SOURCE_DIR}/docs/.doxybook/doxybook2
            --config ${CMAKE_SOURCE_DIR}/docs/.doxybook/config.json
            --input ${CMAKE_CURRENT_BINARY_DIR}/doc_doxygen/zh
            --output ${CMAKE_CURRENT_BINARY_DIR}/docs/zh
            DEPENDS doc_doxygen
            )

        add_custom_target( doxybook2-en  
            COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/docs/en
            COMMAND ${CMAKE_SOURCE_DIR}/docs/.doxybook/doxybook2
            --config ${CMAKE_SOURCE_DIR}/docs/.doxybook/config.en.json
            --input ${CMAKE_CURRENT_BINARY_DIR}/doc_doxygen/en
            --output ${CMAKE_CURRENT_BINARY_DIR}/docs/en
            DEPENDS doc_doxygen_en
            )
        
        add_custom_target( doxy  DEPENDS doxybook2 doxybook2-en
            COMMAND cp -r ${CMAKE_SOURCE_DIR}/docs/.vuepress ${CMAKE_CURRENT_BINARY_DIR}/docs
            COMMAND cp -r ${CMAKE_SOURCE_DIR}/docs/fig ${CMAKE_CURRENT_BINARY_DIR}/docs/zh
            COMMAND cp -r ${CMAKE_SOURCE_DIR}/docs/fig ${CMAKE_CURRENT_BINARY_DIR}/docs/en
            COMMAND cp ${CMAKE_SOURCE_DIR}/docs/en/index.md ${CMAKE_CURRENT_BINARY_DIR}/docs/en/index.md
            COMMAND cp ${CMAKE_SOURCE_DIR}/docs/zh/index.md ${CMAKE_CURRENT_BINARY_DIR}/docs/zh/index.md
            )


    else (DOXYGEN_FOUND)
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND)


    ## Install
    install(TARGETS zeroerr DESTINATION lib)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/zeroerr.hpp DESTINATION include)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION include/..)
else()
    # Export Headerfiles 
    set(ZEROERR_INCLUDE_DIR ${header_dirs} PARENT_SCOPE)
    message(STATUS "ZEROERR_INCLUDE_DIR: ${header_dirs}")
endif()