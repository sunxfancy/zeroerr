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

option(ENABLE_FUZZING "Enable fuzzing(ON, OFF)" OFF)

if(ENABLE_FUZZING)
    message(STATUS "Enable fuzzing")
    if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message(FATAL_ERROR "Fuzzing is only supported by Clang")
    endif()
    add_definitions(-DZEROERR_ENABLE_FUZZING)
endif()

option(BUILD_EXAMPLES "Build examples(ON, OFF)" OFF)
option(BUILD_DOC "Build documentation" OFF)
option(BUILD_TEST "Build unittest" OFF)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:Clang>:-fstandalone-debug>")
# add_compile_options("$<$<CXX_COMPILER_ID:GNU>:-ftime-report>")

set(header_dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/extension
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
    ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${header_dirs})

add_subdirectory(include)
add_subdirectory(src)

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
    if(BUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()

    if(BUILD_TEST)
        add_subdirectory(test)
    endif()

    # Single Header
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/zeroerr.hpp
        DEPENDS
        ${include_files}
        ${source_files}
        ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen-single-file.cmake
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/scripts/gen-single-file.cmake
        COMMENT "assembling the single header")

    add_custom_target(assemble_single_header ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/zeroerr.hpp)

    # Documentation
    find_package(Doxygen)

    if(DOXYGEN_FOUND)
        # request to configure the file
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.en.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.en @ONLY)
        message("Doxygen build started")

        # note the option ALL which allows to build the docs together with the application
        add_custom_target(doc_doxygen
            COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/doc_doxygen/zh
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)

        add_custom_target(doc_doxygen_en
            COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/doc_doxygen/en
            COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.en
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)

        add_custom_target(doxybook2
            COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/docs/zh
            COMMAND ${CMAKE_SOURCE_DIR}/docs/.doxybook/doxybook2
            --config ${CMAKE_SOURCE_DIR}/docs/.doxybook/config.json
            --input ${CMAKE_CURRENT_BINARY_DIR}/doc_doxygen/zh
            --output ${CMAKE_CURRENT_BINARY_DIR}/docs/zh
            DEPENDS doc_doxygen
        )

        add_custom_target(doxybook2-en
            COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/docs/en
            COMMAND ${CMAKE_SOURCE_DIR}/docs/.doxybook/doxybook2
            --config ${CMAKE_SOURCE_DIR}/docs/.doxybook/config.en.json
            --input ${CMAKE_CURRENT_BINARY_DIR}/doc_doxygen/en
            --output ${CMAKE_CURRENT_BINARY_DIR}/docs/en
            DEPENDS doc_doxygen_en
        )

        if(BUILD_DOC)
            set(config_build_doc ALL)
        else()
            set(config_build_doc "")
        endif()

        add_custom_target(doxy ${config_build_doc} DEPENDS doxybook2 doxybook2-en
            COMMAND cp -r ${CMAKE_SOURCE_DIR}/docs/.vuepress ${CMAKE_CURRENT_BINARY_DIR}/docs
            COMMAND cp -r ${CMAKE_SOURCE_DIR}/docs/fig ${CMAKE_CURRENT_BINARY_DIR}/docs/zh
            COMMAND cp -r ${CMAKE_SOURCE_DIR}/docs/fig ${CMAKE_CURRENT_BINARY_DIR}/docs/en
            COMMAND cp ${CMAKE_SOURCE_DIR}/docs/en/index.md ${CMAKE_CURRENT_BINARY_DIR}/docs/en/index.md
            COMMAND cp ${CMAKE_SOURCE_DIR}/docs/zh/index.md ${CMAKE_CURRENT_BINARY_DIR}/docs/zh/index.md
        )

    else(DOXYGEN_FOUND)
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif(DOXYGEN_FOUND)

    # # Install
    install(TARGETS zeroerr DESTINATION lib)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/zeroerr.hpp DESTINATION include)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include DESTINATION include/..)
else()
    # Export Headerfiles
    set(ZEROERR_INCLUDE_DIR ${header_dirs} PARENT_SCOPE)
    message(STATUS "ZEROERR_INCLUDE_DIR: ${header_dirs}")
endif()