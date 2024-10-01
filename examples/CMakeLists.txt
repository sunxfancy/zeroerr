macro(define_example name)
    add_executable(${name} ${CMAKE_CURRENT_SOURCE_DIR}/${name}.cpp)
    add_dependencies(${name} assemble_single_header)
endmacro(define_example)

define_example(1_basic)
define_example(2_log)
define_example(3_fuzzing)
