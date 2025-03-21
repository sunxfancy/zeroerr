set(my_include_folder "${CMAKE_CURRENT_LIST_DIR}/../include/zeroerr")
set(my_extension_folder "${CMAKE_CURRENT_LIST_DIR}/../extension/zeroerr")
set(my_src_folder "${CMAKE_CURRENT_LIST_DIR}/../src")

function(loadfile)
    file(READ ${ARGV0} CONTENTS)
    string(REGEX REPLACE "#include [\"<]zeroerr[^\n]*" "" STRIPPED "${CONTENTS}")
    set(${ARGV1} "${STRIPPED}" PARENT_SCOPE)
endfunction()

loadfile(${my_include_folder}/internal/config.h config)
loadfile(${my_include_folder}/internal/console.h console)
loadfile(${my_include_folder}/internal/debugbreak.h debugbreak)
loadfile(${my_include_folder}/internal/decomposition.h decomposition)
loadfile(${my_include_folder}/internal/rng.h rng)
loadfile(${my_include_folder}/internal/threadsafe.h threadsafe)
loadfile(${my_include_folder}/internal/typetraits.h typetraits)
loadfile(${my_include_folder}/internal/serialization.h serialization)

loadfile(${my_include_folder}/domains/domain.h domain)
loadfile(${my_include_folder}/domains/in_range.h in_range)
loadfile(${my_include_folder}/domains/element_of.h element_of)
loadfile(${my_include_folder}/domains/container_of.h container_of)
loadfile(${my_include_folder}/domains/aggregate_of.h aggregate_of)
loadfile(${my_include_folder}/domains/arbitrary.h arbitrary)

loadfile(${my_include_folder}/assert.h assert)
loadfile(${my_include_folder}/benchmark.h benchmark)
loadfile(${my_include_folder}/color.h color)
loadfile(${my_include_folder}/print.h print)
loadfile(${my_include_folder}/format.h format)
loadfile(${my_include_folder}/dbg.h dbg)
loadfile(${my_include_folder}/log.h log)
loadfile(${my_include_folder}/table.h table)
loadfile(${my_include_folder}/unittest.h unittest)
loadfile(${my_include_folder}/fuzztest.h fuzztest)

loadfile(${my_src_folder}/rng.cpp rng_cpp)
loadfile(${my_src_folder}/benchmark.cpp benchmark_cpp)
loadfile(${my_src_folder}/color.cpp color_cpp)
loadfile(${my_src_folder}/print.cpp print_cpp)
loadfile(${my_src_folder}/console.cpp console_cpp)
loadfile(${my_src_folder}/log.cpp log_cpp)
loadfile(${my_src_folder}/table.cpp table_cpp)
loadfile(${my_src_folder}/unittest.cpp unittest_cpp)
loadfile(${my_src_folder}/fuzztest.cpp fuzztest_cpp)
loadfile(${my_src_folder}/serialization.cpp serialization_cpp)


file(WRITE  zeroerr.hpp "// ======================================================================\n")
file(APPEND zeroerr.hpp "// == DO NOT MODIFY THIS FILE BY HAND - IT IS AUTO GENERATED BY CMAKE! ==\n")
file(APPEND zeroerr.hpp "// ======================================================================\n")
file(APPEND zeroerr.hpp "${config}\n${color}\n${console}\n${debugbreak}\n${threadsafe}\n${typetraits}\n${serialization}\n${print}\n${decomposition}\n${rng}\n")
file(APPEND zeroerr.hpp "${domain}\n${in_range}\n${element_of}\n${container_of}\n${aggregate_of}\n${arbitrary}\n")
file(APPEND zeroerr.hpp "${benchmark}\n${assert}\n${dbg}\n${format}\n${log}\n${table}\n${profiler}\n${unittest}\n${fuzztest}\n")
file(APPEND zeroerr.hpp "#ifdef ZEROERR_IMPLEMENTATION\n")
file(APPEND zeroerr.hpp "${rng_cpp}\n${color_cpp}\n${print_cpp}\n${console_cpp}\n${log_cpp}\n${table_cpp}\n${unittest_cpp}\n${fuzztest_cpp}\n${serialization_cpp}\n${benchmark_cpp}\n")
file(APPEND zeroerr.hpp "#endif // ZEROERR_IMPLEMENTATION\n")
