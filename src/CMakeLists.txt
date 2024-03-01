
set(SOURCE_FILES
    benchmark.cpp
    color.cpp
    console.cpp
    fuzztest.cpp
    log.cpp
    print.cpp
    table.cpp
    unittest.cpp)

# Add the library
add_library(zeroerr STATIC ${SOURCE_FILES})

target_compile_options(zeroerr PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
)