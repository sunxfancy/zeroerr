"""End-to-end tests against real Clang profiles, not mocked counter data."""
import json
from pathlib import Path
import subprocess
import sys
import tempfile

binary, reporter, profdata, cov = sys.argv[1:]


def call(args, expected=0):
    p = subprocess.run(args, capture_output=True, text=True)
    assert p.returncode == expected, (args, p.returncode, p.stdout, p.stderr)
    return p


with tempfile.TemporaryDirectory() as tmp:
    root = Path(tmp)
    collected = root / "collection"
    call([binary, "--coverage-output=" + str(collected)], expected=1)
    rows = [json.loads(s) for s in (collected / "manifest.jsonl").read_text().splitlines()]
    assert len(rows) == 10  # Eight executed tests plus header/footer; skip is absent.
    output = root / "report.json"
    command = [sys.executable, reporter, str(collected), "--binary", binary,
               "--include", r"/coverage_subject\.cpp$", "--llvm-profdata", profdata,
               "--llvm-cov", cov, "--output", str(output)]
    call(command)
    report = json.loads(output.read_text())
    tests = {t["name"]: t for t in report["tests"]}
    positive, duplicate, negative = (tests[n] for n in ('positive "quoted"', "positive duplicate", "negative"))
    assert positive["coverage"] == duplicate["coverage"]
    assert positive["coverage"]["branch_outcomes"] != negative["coverage"]["branch_outcomes"]
    for metric in report["metrics"]:
        assert set(tests["nested"]["coverage"][metric]) == (
            set(positive["coverage"][metric]) | set(negative["coverage"][metric]))
    assert tests["exception"]["result"] == 2
    assert report["identical_coverage_candidates"] == [[positive["id"], duplicate["id"]]]
    conditions = [tests[n]["coverage"]["branch_outcomes"] for n in
                  ("condition true", "condition false", "condition short circuit")]
    assert len({tuple(c) for c in conditions}) == 3
    assert all(s["covered"] == s["total"] for s in report["summary"].values())
    assert any(s["unique"] > 0 for s in tests["condition false"]["stats"].values())
    assert "no instrumented code" in call(command + ["--exclude", ".*"], expected=2).stderr
    # Reusing a directory must fail before running tests or corrupting snapshots.
    original = (collected / "manifest.jsonl").read_bytes()
    call([binary, "--coverage-output=" + str(collected)], expected=2)
    assert (collected / "manifest.jsonl").read_bytes() == original
    listed = root / "listing"
    call([binary, "--list-test-cases", "--coverage-output=" + str(listed)])
    assert not listed.exists()
    filtered = root / "filtered"
    call([binary, "--testcase=negative", "--coverage-output=" + str(filtered)])
    assert len(list(filtered.glob("*.profraw"))) == 1
    empty = root / "empty"
    call([binary, "--testcase=does-not-exist", "--coverage-output=" + str(empty)])
    empty_command = command.copy()
    empty_command[2] = str(empty)
    assert "no tests" in call(empty_command, expected=2).stderr
    # Public one-command interface: default source scope, forwarded filters,
    # output paths containing shell metacharacters, and preserved failure status.
    convenient = root / "one command & report"
    call([binary, "-c", "--testcase=positive .*", "--coverage-output=" + str(convenient)])
    automatic_report = json.loads((convenient / "report.json").read_text())
    assert len(automatic_report["tests"]) == 2
    assert len(automatic_report["identical_coverage_candidates"]) == 1
    assert (convenient / "report.summary.txt").exists()
    assert all("coverage_subject.cpp" in feature[0]
               for features in automatic_report["features"].values() for feature in features)
    failed_run = root / "failed-run"
    call([binary, "--coverage", "--coverage-output=" + str(failed_run)], expected=1)
    assert (failed_run / "report.json").exists()
    call([binary, "--coverage", "--list-test-cases", "--coverage-output=" + str(listed)])
    assert not listed.exists()
    assert "already exists" in call([binary, "-c", "--coverage-output=" + str(convenient)], expected=2).stderr
    # A crash/interruption cannot silently produce an apparently complete report.
    (collected / "manifest.jsonl").write_text("\n".join(json.dumps(r) for r in rows[:-1]))
    assert "incomplete" in call(command, expected=2).stderr
print("Clang per-test coverage integration passed")
