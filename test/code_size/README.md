# Diagnostic text code-size comparison

The internal append-only `TextBuffer` uses 256 bytes of inline storage, grows
geometrically, and preserves embedded NUL bytes. `TextStream` adapts it to
`std::ostream`, so existing stream insertion and `PrinterExt` customization
continue to work. Only concrete value dispatch stays in template callbacks;
stream construction, printing order, cleanup, parsing and concatenation live in
`src/text.cpp`. Color-dependent assertion log assembly lives in the separate
`src/assert_text.cpp` library member.

Public return types and assertion fields remain `std::string`. The public default
`Printer` retains its existing stringstream behavior, including clearing and
appending modes. This change does not eliminate all standard-string or iostream
code from an application. Format strings with too few arguments now throw
`std::invalid_argument` instead of accessing beyond the argument array. Zero-argument
formatting is also supported without a zero-length array. Other existing parsing
behavior, including ignoring surplus arguments, is preserved.

## Reproduce

The baseline is commit `43c31f64ca1144ca8dd3aad36ce2915136dcab0d`. Copy this
`test/code_size` directory into that revision as well, so both revisions use the
identical workload and measurement script. Use separate build directories and
the same toolchain and configuration. From each repository root:

```powershell
cmake -S test/code_size -B build-probe -G "Visual Studio 18 2026" -A x64
cmake --build build-probe --config Release -- /m:1
python test/code_size/measure.py build-probe Release
./build-probe/Release/code_size_probe.exe
cmake --build build-probe --config Debug --clean-first -- /m:1
python test/code_size/measure.py build-probe Debug
./build-probe/Debug/code_size_probe.exe
```

The probe instantiates formatting and expression decomposition with 24 distinct
streamable operand types. Both configurations and revisions produced checksum
`3095476972` with no command-line arguments. Avoid accumulating incremental PDB
updates when comparing debug information: measure a clean Debug build.

For the existing test executable, build each revision with the same test sources:

```powershell
cmake -S . -B build-tests -G "Visual Studio 18 2026" -A x64 -DBUILD_TEST=ON -DUSE_MOLD=OFF
cmake --build build-tests --config Release -- /m:1
python test/code_size/measure.py build-tests Release unittest
ctest --test-dir build-tests/test -C Release --output-on-failure
```

The new boundary tests live in a separate `text_test` executable, keeping the
existing `unittest` workload unchanged. `measure.py` reads Windows PE/COFF section
headers without requiring dumpbin. It reports file sizes separately from `.text`
section sizes; object sections may contain duplicate code removed by the linker.
Debug `.text` can include incremental-link padding. Library object totals include
the new shared implementation. The script is intended for the MSVC artifacts
used here, rather than ELF, Mach-O or arbitrary bigobj files.

## First implementation (historical)

See `results-msvc.json` for raw bytes and percentage changes, measured with MSVC
19.51.36248.0, x64, C++17 and CMake's default Release/Debug flags. These measurements
describe these workloads and this compiler; they are not general size guarantees.

- Existing Release `unittest`: EXE 916,992 -> 832,000 bytes (-9.27%);
  `.text` 721,425 -> 636,721 bytes (-11.74%). Its test object files' combined
  `.text` shrank by 12.71%.
- Release probe: caller object 710,020 -> 471,995 bytes (-33.52%); caller object
  `.text` shrank by 23.39%. The final EXE grew 45,568 -> 57,856 bytes (+26.97%),
  with `.text` growing by 42.77%. Reduced template output alone does not guarantee
  a smaller linked binary; shared implementation and linker optimization costs
  matter for small applications.
- Debug probe: EXE 267,776 -> 240,128 bytes (-10.33%); `.text` shrank by 11.37%.
  Its clean-build PDB grew 1,863,680 -> 1,970,176 bytes (+5.71%).
- Library object totals grew by about 0.75% in Release and 1.37% in Debug.
  The shared buffer implementation has a fixed cost, so code and debug-information
  reductions should be evaluated in the actual consuming application.

## Optimized shared implementation

The follow-up compared the identical workload with the original implementation,
the first TextBuffer implementation, and the optimized version. The JSON retains
the first two measurements as `before` and `after`, and adds `optimized` plus
percentage changes against both. Compiler optimization flags were unchanged;
`/MAP` was added only to obtain link attribution.

The link maps identify the main regression: each of the 24 `exercise<N>` public
symbol spans grew from 592 to 1,200 bytes in the first version. The original
format and comparison template families each had 24 symbols sharing one linked
address; neither family remained out of line in the first version. The smaller
wrappers had encouraged inlining into each caller, so a smaller object file
produced a larger final binary. The first version's map attributes 33,616 bytes
to main.obj public-symbol spans versus 24,501 originally, and another 3,228 to
the shared text implementation. These are address spans including padding and
possibly private symbols, not exact instruction counts.

There was also an unnecessary archive dependency. `dumpbin /symbols text.obj`
showed undefined `zeroerr::Dim` and `zeroerr::Reset` references from assertion-log
formatting. Even though that unused function was discarded, extracting text.obj
had pulled in color.obj and its global initialization. The optimized probe's map
contains no color.obj. The raw map summaries are in `link-map-summary-msvc.json`.

The fixes are:

- Borrow values through synchronous printing callbacks and construct/destroy the
  stream in the common backend. `std::addressof` and cv-preserving casts support
  overloaded address-of operators and volatile operands without copying them
  into separate type-erasure storage.
- Keep the format and comparison entry points out of line with the internal
  `ZEROERR_DIAGNOSTIC_NOINLINE` attribute. This restores linker folding: the
  optimized map again shows 24 format symbols at one address and 24 comparison
  symbols at one address. Each `exercise<N>` span is now 656 bytes.
- Use plain, non-virtual byte storage for concatenation and parsing results.
  Only rendering constructs a streambuf adapter; `TextStream` contains an
  ordinary ostream rather than deriving another stream class.
- Put color-dependent assertion log assembly in its own source file so format
  and comparison users do not extract its library member.

Final measurements with the same compiler and workloads:

- Release probe EXE: 57,856 -> 43,520 bytes (-24.78% against the first version,
  -4.49% against the original). Its `.text`: 41,336 -> 28,200 bytes (-31.78%,
  and -2.60% against the original). Caller object `.text` is 40.05% below original.
- Existing Release unittest EXE: 832,000 -> 818,176 bytes (-1.66% against the
  first version, -10.78% against original). Its `.text` is 13.52% below original.
- Debug probe EXE: 240,128 -> 232,960 bytes (-2.99% against the first version,
  -13.00% against original). Its clean-build PDB is 1,953,792 bytes, 4.84% above
  original; the debug-information cost is reported separately from code.

The callbacks and noinline boundaries trade some optimization freedom and calls
for less repeated code. Runtime performance has not been benchmarked. The public
Printer representation, function signatures, string return types, argument
printing order (including surplus arguments), and custom printing remain intact.

To reproduce map attribution, add `/MAP` to the same Release linker flags and
inspect the probe map; this does not disable normal dead stripping or ICF:

```powershell
cmake -S test/code_size -B build-probe -G "Visual Studio 18 2026" -A x64 -DCMAKE_EXE_LINKER_FLAGS_RELEASE="/INCREMENTAL:NO /MAP"
cmake --build build-probe --config Release -- /m:1
python test/code_size/summarize-map.py build-probe/Release/code_size_probe.map
```

## Verification

Release tests passed with the same existing outcome as the baseline: 50 passed,
15 warning cases, zero failures and two skipped cases. All seven new text tests
passed in Release and Debug. They cover growth, self-appends across reallocation,
embedded NUL bytes, stream formatting, custom insertion and `PrinterExt`, placeholder
bounds, comparison chains, logical joins and public Printer/assertion output, plus
callback lifetimes, cv qualifiers, overloaded address-of and surplus argument order.

The full default Debug test build fails in the baseline's bundled fmt because
`stdext::checked_array_iterator` is unavailable in this MSVC version. The new text
tests were run separately; their Debug link used `/INCREMENTAL:NO` after the
incremental linker rejected an object section. The probe comparison uses matching
default Debug flags on both sides. A generated single header was also compiled
as separate implementation and consumer translation units, producing the same
probe checksum.

## Internal String and lightweight headers

The next stage replaces core string storage with `zeroerr::String` and Printer's
stream with `StringBuffer`, and makes optional umbrella modules explicit. On the
same MSVC Release probe, checksum remains `3095476972`. EXE size is 46,592 bytes
(+2.25% against original 45,568; +7.06% against optimized 43,520). PE text is
29,032 bytes (+0.28% against original 28,952; +2.95% against optimized 28,200).
This stage establishes the consumer dependency boundary; it does not improve the
previous stage's final EXE size. The probe explicitly opts into standard stream
adaptation to keep its custom stream insertion workload comparable.

The final Release root CTest suite covers the existing tests, text rendering,
internal String, single-header core/XML and optional modules. The standalone
lightweight suite also passes in Debug. Header dependency guards apply to normal
consumer units; the implementation unit intentionally permits standard runtime
facilities, as documented in `docs/dependencies.md`.
