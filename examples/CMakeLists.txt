add_executable(1_basic ${CMAKE_CURRENT_SOURCE_DIR}/1_basic.cpp)
add_executable(2_log ${CMAKE_CURRENT_SOURCE_DIR}/2_log.cpp)
add_dependencies(1_basic assemble_single_header)
add_dependencies(2_log assemble_single_header)