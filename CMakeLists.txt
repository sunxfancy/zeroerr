cmake_minimum_required(VERSION 3.15)

project(ZeroErr LANGUAGES C CXX)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

set(COLORFUL_OUTPUT "AUTO" CACHE STRING "Use colorful output(ON, OFF, AUTO)")
set_property(CACHE COLORFUL_OUTPUT PROPERTY STRINGS AUTO ON OFF)
if(NOT COLORFUL_OUTPUT MATCHES "^(AUTO|ON|OFF)$")
    message(FATAL_ERROR "COLORFUL_OUTPUT must be AUTO, ON, or OFF")
endif()

if(COLORFUL_OUTPUT STREQUAL "ON")
    add_definitions(-DZEROERR_ALWAYS_COLORFUL)
endif()

if(COLORFUL_OUTPUT STREQUAL "OFF")
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

option(USE_MOLD "Use mold for linking" ON)

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
endif()

option(BUILD_EXAMPLES "Build examples(ON, OFF)" OFF)
option(BUILD_DOC "Build documentation" OFF)
option(BUILD_TEST "Build unittest" OFF)
option(ZEROERR_ENABLE_COVERAGE "Enable Clang per-test source coverage" OFF)

if(ZEROERR_ENABLE_COVERAGE)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|AppleClang)$")
        message(FATAL_ERROR "zeroerr coverage requires Clang or AppleClang")
    endif()
    find_package(Python3 3.9 REQUIRED COMPONENTS Interpreter)
    get_filename_component(coverage_compiler_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
    string(REGEX MATCH "^[0-9]+" coverage_llvm_major "${CMAKE_CXX_COMPILER_VERSION}")
    find_program(ZEROERR_LLVM_PROFDATA NAMES llvm-profdata-${coverage_llvm_major} llvm-profdata
        HINTS "${coverage_compiler_dir}")
    find_program(ZEROERR_LLVM_COV NAMES llvm-cov-${coverage_llvm_major} llvm-cov
        HINTS "${coverage_compiler_dir}")
    if(NOT ZEROERR_LLVM_PROFDATA OR NOT ZEROERR_LLVM_COV)
        message(FATAL_ERROR "Coverage needs llvm-profdata and llvm-cov matching Clang; set ZEROERR_LLVM_PROFDATA/ZEROERR_LLVM_COV")
    endif()
endif()

# Apply this helper to separately compiled production libraries as well.
function(zeroerr_instrument_coverage target)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "^(Clang|AppleClang)$")
        message(FATAL_ERROR "zeroerr coverage requires Clang or AppleClang")
    endif()
    target_compile_options(${target} PUBLIC -fprofile-instr-generate -fcoverage-mapping
        -fprofile-update=atomic)
    target_link_options(${target} PUBLIC -fprofile-instr-generate)
endfunction()

# Call after target_link_libraries(my_tests PRIVATE zeroerr).
# The generated target builds, collects, analyzes, and prints a report in one command.
function(zeroerr_add_coverage target)
    if(NOT TARGET zeroerr OR NOT TARGET ${target})
        message(FATAL_ERROR "zeroerr_add_coverage requires zeroerr and an existing test target")
    endif()
    get_target_property(coverage_python zeroerr ZEROERR_COVERAGE_PYTHON_PATH)
    if(NOT coverage_python)
        message(FATAL_ERROR "zeroerr_add_coverage requires ZEROERR_ENABLE_COVERAGE=ON")
    endif()
    cmake_parse_arguments(COVERAGE "" "INCLUDE" "EXCLUDE;ARGS" ${ARGN})
    if(COVERAGE_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "Unknown zeroerr_add_coverage arguments: ${COVERAGE_UNPARSED_ARGUMENTS}")
    endif()
    get_target_property(coverage_script zeroerr ZEROERR_COVERAGE_SCRIPT_PATH)
    get_target_property(coverage_profdata zeroerr ZEROERR_COVERAGE_PROFDATA_PATH)
    get_target_property(coverage_cov zeroerr ZEROERR_COVERAGE_COV_PATH)
    set(coverage_options)
    if(DEFINED COVERAGE_INCLUDE)
        list(APPEND coverage_options --include "${COVERAGE_INCLUDE}")
    endif()
    foreach(exclude IN LISTS COVERAGE_EXCLUDE)
        list(APPEND coverage_options --exclude "${exclude}")
    endforeach()
    add_custom_target(${target}-coverage
        COMMAND "${coverage_python}" "${coverage_script}" --run $<TARGET_FILE:${target}>
            --source-root "${CMAKE_SOURCE_DIR}"
            --llvm-profdata "${coverage_profdata}" --llvm-cov "${coverage_cov}"
            ${coverage_options} -- ${COVERAGE_ARGS}
        DEPENDS ${target}
        WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
        USES_TERMINAL VERBATIM)
endfunction()

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
        enable_testing()
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

        if(BUILD_DOC)
            set(config_build_doc ALL)
        else()
            set(config_build_doc "")
        endif()

        add_custom_target(doxy ${config_build_doc} DEPENDS doc_doxygen doc_doxygen_en
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/docs/fig ${CMAKE_CURRENT_BINARY_DIR}/doc_doxygen/fig)

    else(DOXYGEN_FOUND)
        message("Doxygen need to be installed to generate the doxygen documentation")
    endif(DOXYGEN_FOUND)

    # # Install
    install(TARGETS zeroerr DESTINATION lib)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/zeroerr.hpp DESTINATION include)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION include)
else()
    # Export Headerfiles
    set(ZEROERR_INCLUDE_DIR ${header_dirs} PARENT_SCOPE)
    message(STATUS "ZEROERR_INCLUDE_DIR: ${header_dirs}")
endif()
