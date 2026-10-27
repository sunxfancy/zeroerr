"""Check actual runner exit codes in both compatible and strict check modes."""
import subprocess
import sys

for runner, strict in ((sys.argv[1], True), (sys.argv[2], False)):
    for name in ("singleEvaluation", "expectedInvariant", "check", "comparison",
                 "checkNot", "checkThrows", "require", "assert", "helper",
                 "helperRequire", "helperAssert", "subcaseHelper"):
        result = subprocess.run(
            [runner, "--testcase=^assertion." + name + "$"],
            capture_output=True, text=True, encoding="utf-8", errors="replace", timeout=30,
        )
        succeeds = name in ("singleEvaluation", "expectedInvariant") or (
            not strict and name in ("check", "comparison", "checkNot", "checkThrows", "helper", "subcaseHelper")
        )
        if (result.returncode == 0) != succeeds:
            raise SystemExit(f"{runner}: {name}: unexpected exit {result.returncode}\n"
                             + result.stdout + result.stderr)
        if not succeeds and "Assertion Failed" not in result.stdout + result.stderr:
            raise SystemExit(f"{name}: missing assertion diagnostic")
print("single evaluation and strict/compatible assertion exit contracts passed")
