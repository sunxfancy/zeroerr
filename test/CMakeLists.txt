## LLVM Support

find_package(LLVM CONFIG)
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

    set(CMAKE_CUDA_ARCHITECTURES 80)

    set(cuda_files cuda_test.cu)
else()
    set(cuda_files "")
endif()

## Test Cases

set(test_files 
    fuzz_test.cpp
    llvm_test.cpp
    log_test.cpp
    print_test.cpp
    table_test.cpp
    unit_test.cpp
)

if (ENABLE_FUZZING)
    set(libfuzzer clang_rt.fuzzer_no_main-x86_64)
else()
    set(libfuzzer "")
endif()

add_executable(unittest ${test_files} ${cuda_files})
target_link_libraries(unittest zeroerr ${libfuzzer} ${llvm_libs})
target_compile_options(unittest PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic>
    )

## Fuzzing Support
if (ENABLE_FUZZING)
    execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-runtime-dir
        OUTPUT_VARIABLE LIB_FUZZER_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set_target_properties(unittest PROPERTIES COMPILE_DEFINITIONS ZEROERR_ENABLE_FUZZING)
    target_compile_options(unittest PRIVATE -fsanitize=fuzzer-no-link)
    target_link_directories(unittest PRIVATE ${LIB_FUZZER_PATH})
    target_link_options(unittest PRIVATE -fsanitize=fuzzer-no-link)
endif()

enable_testing()
add_test(NAME RunUnitTest COMMAND unittest)