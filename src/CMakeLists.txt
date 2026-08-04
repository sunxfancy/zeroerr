
set(source_files
    ${CMAKE_CURRENT_SOURCE_DIR}/rng.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/benchmark.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/color.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/console.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fuzztest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/print.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/table.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/unittest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/serialization.cpp)

set(source_files ${source_files} PARENT_SCOPE)

# Add the library
add_library(zeroerr STATIC ${source_files})

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
