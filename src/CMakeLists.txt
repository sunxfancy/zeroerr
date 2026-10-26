
set(source_files
    ${CMAKE_CURRENT_SOURCE_DIR}/threadsafe.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/buffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/string.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/rng.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/benchmark.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/color.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/console.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fuzztest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/assert_text.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/text.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/print.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/table.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unittest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/serialization.cpp)

set(source_files ${source_files} PARENT_SCOPE)

# Add the library
add_library(zeroerr STATIC ${source_files})

if(ZEROERR_ENABLE_COVERAGE)
    target_compile_features(zeroerr PRIVATE cxx_std_17)
    target_compile_definitions(zeroerr PRIVATE ZEROERR_ENABLE_COVERAGE)
    zeroerr_instrument_coverage(zeroerr)
    target_compile_definitions(zeroerr PRIVATE
        ZEROERR_COVERAGE_PYTHON="${Python3_EXECUTABLE}"
        ZEROERR_COVERAGE_SCRIPT="${PROJECT_SOURCE_DIR}/scripts/coverage-report.py"
        ZEROERR_COVERAGE_SOURCE_ROOT="${CMAKE_SOURCE_DIR}"
        ZEROERR_COVERAGE_PROFDATA="${ZEROERR_LLVM_PROFDATA}"
        ZEROERR_COVERAGE_COV="${ZEROERR_LLVM_COV}")
    set_target_properties(zeroerr PROPERTIES
        ZEROERR_COVERAGE_PYTHON_PATH "${Python3_EXECUTABLE}"
        ZEROERR_COVERAGE_SCRIPT_PATH "${PROJECT_SOURCE_DIR}/scripts/coverage-report.py"
        ZEROERR_COVERAGE_PROFDATA_PATH "${ZEROERR_LLVM_PROFDATA}"
        ZEROERR_COVERAGE_COV_PATH "${ZEROERR_LLVM_COV}")
endif()

target_include_directories(zeroerr PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../extension
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party
    ${CMAKE_CURRENT_BINARY_DIR}/..
)

target_compile_options(zeroerr PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /utf-8>
    $<$<C_COMPILER_ID:MSVC>:/utf-8>
    $<$<CXX_COMPILER_ID:Clang>:-fstandalone-debug>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)

if (ENABLE_FUZZING)
    target_compile_definitions(zeroerr PUBLIC ZEROERR_ENABLE_FUZZING)
endif()
