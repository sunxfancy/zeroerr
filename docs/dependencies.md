# Header dependencies and optional modules

A normal translation unit may include `zeroerr.hpp` (or modular `zeroerr.h`) without defining `ZEROERR_IMPLEMENTATION`. Assertions, logging, printing and unit tests are available by default. Core text storage uses `zeroerr::String`; `Printer::os` is a `zeroerr::StringBuffer`.

Default consumer headers do not include the implementations of `<string>`, `<ostream>`, `<iostream>`, `<sstream>`, `<chrono>`, `<regex>`, `<mutex>` or `<complex>`. This is not a promise of no standard library: core declarations still use lightweight facilities including containers, tuples, memory, type traits, atomics and exceptions. `<iosfwd>` supplies forward declarations for the reporter interface. Transitive dependencies can differ by standard-library vendor; `test/lightweight/forbidden_headers.h` checks the supported header guards to catch regressions.

Define the following macros before including the umbrella header to enable optional modules:

- `ZEROERR_ENABLE_BENCHMARK`: benchmarking (including its table rendering implementation).
- `ZEROERR_ENABLE_FUZZTEST`: fuzz testing, domains and serialization.
- `ZEROERR_ENABLE_TABLE`: tables.
- `ZEROERR_ENABLE_STD_ADAPTER`: standard stream insertion and common manipulators. This explicitly includes string and stream headers.

With modular headers, explicitly include `zeroerr/benchmark.h`, `zeroerr/fuzztest.h`, `zeroerr/table.h` or `zeroerr/std_adapter.h` instead. Enable optional modules consistently where their definitions are needed, including the single implementation translation unit. Defining `ZEROERR_IMPLEMENTATION` alone does not enable optional modules.

## Runtime boundary

Exactly one translation unit may define `ZEROERR_IMPLEMENTATION` before including `zeroerr.hpp`, or the application can link the compiled library. This implementation is allowed to include standard-library runtime facilities: streams and strings for test capture, reports and file output; regex for test filtering; chrono for timing; mutex for synchronization; and standard exceptions for error reporting. These dependencies are intentionally confined to implementation sources. Optional fuzz/benchmark/table modules may add further dependencies when explicitly enabled.

## Compatibility and migration

Common assertion, log, test and formatting call syntax is retained. String results and stored names/messages now use `zeroerr::String`, which provides `data()`, `size()`, `c_str()`, concatenation and common search operations. Conversions to and from compatible owning string types are templates; applications using `std::string` must include `<string>` themselves. This is a source migration for explicitly typed callbacks, containers and reference bindings, and an ABI change: rebuild all library consumers together.

`Printer` captures into its own `StringBuffer` by default. `Printer(stream)` still attaches a supplied stream with `write` and `flush` methods; the caller includes that stream's headers. The legacy `use_stringstream` flag now indicates capture mode. Extensions using `P.os << value` work for core supported values. Include `zeroerr/std_adapter.h` for standard stream-only custom types and common manipulators. `StringBuffer` is not a complete `std::ostream`: native `std::ostream&` APIs need an explicit temporary stream and then a buffer write; locale and all standard formatting flags are not emulated.

Public timing fields use `zeroerr::Duration` (seconds, with `count()`) and `zeroerr::Timestamp` (nanoseconds). Code explicitly naming their former chrono types must adapt. Core type definitions do not depend on which standard headers the application included first.

## Verification

`cmake -S test/lightweight -B build-lightweight`, followed by a build and CTest, checks modular and single-header consumers, include order, internal string behavior, XML reporting and explicitly enabled optional modules. The runtime translation unit is deliberately outside the forbidden-header check.

For libc++ versions with compatibility transitive includes, core headers temporarily set `_LIBCPP_REMOVE_TRANSITIVE_INCLUDES` while loading their explicit standard dependencies, then restore the caller's setting with `push_macro`/`pop_macro`. This suppresses incidental compatibility includes without changing library types or the caller's configuration.
