"""Compile warning regressions with --compiler clang++, g++, or cl (developer shell)."""
import argparse
from pathlib import Path
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--compiler", required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    msvc = Path(args.compiler).stem.lower() == "cl"
    version = subprocess.run([args.compiler, "--version"], capture_output=True, text=True,
                             errors="replace")
    clang = "clang" in version.stdout.lower()
    with tempfile.TemporaryDirectory() as tmp:
        source = Path(tmp) / "probe.cpp"
        def compile_probe(code, flags, diagnostic=None):
            source.write_text(code, encoding="utf-8")
            if msvc:
                command = [args.compiler, "/nologo", "/std:c++17", "/EHsc", "/utf-8", "/c",
                           f"/I{root / 'include'}", f"/I{root}", f"/Fo{tmp}/probe.obj"]
            else:
                command = [args.compiler, "-std=c++17", "-fsyntax-only",
                           f"-I{root / 'include'}", f"-I{root}"]
            result = subprocess.run(command + flags + [str(source)], capture_output=True,
                                    text=True, errors="replace")
            output = result.stdout + result.stderr
            if diagnostic is None:
                assert result.returncode == 0, output
            else:
                assert result.returncode != 0 and diagnostic in output, output

        # A consumer's loop variable must not collide with assertion/logging internals.
        for header in ("zeroerr.h", "zeroerr.hpp"):
            compile_probe(f'#include "{header}"\n'
                          + ('' if msvc else '#pragma GCC diagnostic error "-Wshadow"\n') +
                          'void probe() { for (int i = 0; i < 2; ++i) { REQUIRE(i >= 0); } }\n',
                          ["/W4", "/we4456", "/we4458"] if msvc else [])
        # Pragmas cannot separate a function/lambda declarator from its body.
        for header in ("zeroerr/fuzztest.h", "zeroerr.hpp"):
            compile_probe(f'#include "{header}"\n'
                          'FUZZ_TEST_CASE("fuzz declaration") {}\n'
                          'TEST_CASE("subcase declaration") { SUB_CASE("child") {}; }\n', [])
        implementation = '#define ZEROERR_IMPLEMENTATION\n#include "zeroerr.hpp"\n'
        deprecated_flags = ["/we4996"] if msvc else ["-Werror=deprecated-declarations"]
        compile_probe(implementation, deprecated_flags)
        compile_probe(implementation +
                      '[[deprecated("user diagnostic")]] void old_api();\n'
                      'void consumer() { old_api(); }\n',
                      deprecated_flags, "4996" if msvc else "deprecated")
        if clang:
            prefix = '#include "zeroerr/internal/config.h"\n'
            scoped = ('ZEROERR_SUPPRESS_VARIADIC_MACRO\n'
                      '#define ZEROERR_PROBE(x, ...) x\n'
                      'int value = ZEROERR_PROBE(1);\n'
                      'ZEROERR_SUPPRESS_VARIADIC_MACRO_POP\n')
            flags = ["-Werror=unknown-warning-option", "-Werror=gnu-zero-variadic-macro-arguments"]
            compile_probe(prefix + scoped, flags)
            # Leaving the scope must restore the caller's diagnostic state.
            compile_probe(prefix + scoped +
                          '#define USER_PROBE(x, ...) x\nint user = USER_PROBE(2);\n',
                          flags, "variadic")
            compile_probe(prefix + scoped +
                          '#pragma clang diagnostic ignored "-Wzeroerr-nonexistent-warning"\n',
                          flags, "unknown warning group")
    print("Warning hygiene probes passed" + (" (including Clang diagnostic restoration)" if clang else ""))


if __name__ == "__main__":
    main()
