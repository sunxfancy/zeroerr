
set(include_files
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/buffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/std_adapter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/internal/time.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/internal/fuzz_signal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/string.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/internal/text.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/internal/typetraits.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/internal/config.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/internal/console.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/internal/debugbreak.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/internal/decomposition.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/internal/rng.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/internal/serialization.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/internal/threadsafe.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/domains/arbitrary.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/assert.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/benchmark.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/color.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/dbg.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/format.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/fuzztest.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/log.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/print.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/table.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/unittest.h
)

set(include_files
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/buffer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/std_adapter.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/internal/time.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/internal/fuzz_signal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/zeroerr/string.h ${include_files} PARENT_SCOPE)