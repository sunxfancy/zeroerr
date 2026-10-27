## LLVM Support

find_package(LLVM CONFIG QUIET)
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
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)
    set(CMAKE_CUDA_ARCHITECTURES 80)

    set(cuda_files cuda_test.cu)
    add_executable(cudatest ${cuda_files})
    target_link_libraries(cudatest zeroerr)
endif()

## Test Cases

set(test_files 
    ${CMAKE_CURRENT_SOURCE_DIR}/fuzz_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/llvm_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/log_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/print_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/table_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unit_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/serialization_test.cpp
)

if (ENABLE_FUZZING)
    set(libfuzzer clang_rt.fuzzer_no_main-x86_64)
else()
    set(libfuzzer "")
endif()

add_executable(unittest ${test_files})
target_link_libraries(unittest zeroerr ${libfuzzer} ${llvm_libs})
target_compile_options(unittest PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /utf-8>
    $<$<C_COMPILER_ID:MSVC>:/utf-8>
    $<$<CXX_COMPILER_ID:Clang>:-fstandalone-debug>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

## Fuzzing Support
if (ENABLE_FUZZING)
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-runtime-dir
        OUTPUT_VARIABLE LIB_FUZZER_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (NOT LIB_FUZZER_PATH)
        message(FATAL_ERROR "libfuzzer not found")
    else()
        message(STATUS "libfuzzer path: ${LIB_FUZZER_PATH}")
    endif()

    set_target_properties(unittest PROPERTIES COMPILE_DEFINITIONS ZEROERR_ENABLE_FUZZING)
    target_compile_options(unittest PRIVATE -fsanitize=fuzzer-no-link)
    target_link_directories(unittest PRIVATE ${LIB_FUZZER_PATH})
    target_link_options(unittest PRIVATE -fsanitize=fuzzer-no-link)
endif()

enable_testing()
add_test(NAME RunUnitTest COMMAND unittest)

# Keep the existing unittest workload unchanged for code-size comparisons.
add_executable(text_test text_test.cpp)
target_link_libraries(text_test PRIVATE zeroerr)
add_test(NAME RunTextTest COMMAND text_test)

add_subdirectory(lightweight)
# Separate translation units ensure neither include order is masked by a PCH.
foreach(order IN ITEMS zeroerr_first stl_first)
    add_executable(print_capabilities_${order} print_capabilities.cpp)
    target_link_libraries(print_capabilities_${order} PRIVATE zeroerr)
    if(order STREQUAL "stl_first")
        target_compile_definitions(print_capabilities_${order} PRIVATE ZEROERR_TEST_STL_FIRST)
    endif()
    if(MSVC)
        target_compile_options(print_capabilities_${order} PRIVATE /permissive-)
    endif()
    add_test(NAME PrintCapabilities_${order} COMMAND print_capabilities_${order})
endforeach()
if(ZEROERR_ENABLE_COVERAGE)
    zeroerr_add_coverage(unittest INCLUDE "/zeroerr/(src|include)/")
    add_executable(coverage_fixture coverage_fixture.cpp coverage_subject.cpp)
    target_link_libraries(coverage_fixture PRIVATE zeroerr)
    add_test(NAME ClangCoverageIntegration COMMAND ${Python3_EXECUTABLE}
        ${CMAKE_CURRENT_SOURCE_DIR}/coverage_integration.py $<TARGET_FILE:coverage_fixture>
        ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/coverage-report.py
        ${ZEROERR_LLVM_PROFDATA} ${ZEROERR_LLVM_COV})
endif()
