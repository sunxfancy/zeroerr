macro(define_example name)
    add_executable(${name} ${CMAKE_CURRENT_SOURCE_DIR}/${name}.cpp)
    add_dependencies(${name} assemble_single_header)
    target_compile_options(${name} PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/W4 /utf-8>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
    )
endmacro(define_example)

define_example(1_basic)
define_example(2_log)
define_example(3_fuzzing)
