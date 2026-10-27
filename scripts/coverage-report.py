#!/usr/bin/env python3
"""Convert zeroerr Clang snapshots into a per-test coverage matrix (Python 3.9+)."""
import argparse
import collections
import hashlib
import json
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import uuid

METRICS = ("functions", "regions", "branch_outcomes")


def run(command):
    result = subprocess.run(command, text=True, encoding="utf-8", capture_output=True)
    if result.returncode:
        raise RuntimeError(f"{' '.join(map(str, command))}\n{result.stderr}")
    # LLVM can return success with a mismatched/partially missing profile.
    if result.stderr.strip():
        raise RuntimeError(f"LLVM diagnostic (refusing a potentially incomplete report):\n{result.stderr}")
    return result.stdout


def extract(export, include, exclude):
    if (export.get("type") != "llvm.coverage.json.export"
            or export.get("version", "").split(".")[0] not in ("2", "3")):
        raise ValueError("unsupported llvm-cov JSON schema (expected version 2.x or 3.x)")
    covered = {m: set() for m in METRICS}
    universe = {m: set() for m in METRICS}

    def selected(path):
        return include.search(path) and not any(p.search(path) for p in exclude)

    def add(metric, key, count):
        universe[metric].add(key)
        if count > 0:
            covered[metric].add(key)

    for data in export["data"]:
        for fn in data["functions"]:
            files = [p.replace("\\", "/") for p in fn["filenames"]]
            name = fn["name"]  # Keep template instantiations distinct.
            regions = fn["regions"]
            if regions:
                primary = files[regions[0][5]]
                if selected(primary):
                    add("functions", (primary, name), fn["count"])
            for r in regions:
                path = files[r[5]]
                if r[7] == 0 and selected(path):  # CodeRegion, not gap/expansion/skipped.
                    add("regions", (path, name, r[5], *r[:4]), r[4])
            # Older exporters may omit branch data. Do not claim zero branches.
            if "branches" not in fn:
                raise ValueError("llvm-cov export lacks branch information; use a newer LLVM")
            for b in fn["branches"]:
                path = files[b[6]]
                if selected(path):
                    for outcome, count in (("true", b[4]), ("false", b[5])):
                        add("branch_outcomes", (path, name, b[6], *b[:4], outcome), count)
    return covered, universe


def analyze(rows, coverage, universe):
    # Shared dictionaries keep the matrix compact even for thousands of tests.
    features = {m: sorted(universe[m]) for m in METRICS}
    indexes = {m: {key: i for i, key in enumerate(features[m])} for m in METRICS}
    frequencies = {m: collections.Counter(k for c in coverage for k in c[m]) for m in METRICS}
    groups = collections.defaultdict(list)
    tests = []
    for i, (row, cov) in enumerate(zip(rows, coverage)):
        item = dict(row, id=i)
        item["coverage"] = {m: sorted(indexes[m][k] for k in cov[m]) for m in METRICS}
        item["stats"] = {
            m: {"covered": len(cov[m]), "total": len(universe[m]),
                "unique": sum(frequencies[m][k] == 1 for k in cov[m])}
            for m in METRICS
        }
        tests.append(item)
        signature = tuple(tuple(item["coverage"][m]) for m in METRICS)
        # Empty scope is not evidence of equivalent tests; failures aren't candidates.
        if any(signature) and row["result"] == 0:
            groups[signature].append(i)
    return {
        "schema": 1,
        "metrics": list(METRICS),
        "uncollected_metrics": ["lines", "conditions", "mcdc", "paths"],
        "feature_layout": {
            "functions": ["file", "mangled_function"],
            "regions": ["file", "mangled_function", "local_file_id", "start_line", "start_column", "end_line", "end_column"],
            "branch_outcomes": ["file", "mangled_function", "local_file_id", "start_line", "start_column", "end_line", "end_column", "outcome"],
        },
        "features": features,
        "tests": tests,
        "summary": {m: {"covered": len(frequencies[m]), "total": len(universe[m])} for m in METRICS},
        "identical_coverage_candidates": [ids for ids in groups.values() if len(ids) > 1],
        "warning": "Identical measured coverage is not semantic equivalence or permission to remove tests.",
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=Path, nargs="?", help="existing collection directory (offline mode)")
    parser.add_argument("--binary", type=Path, help="exact executable used for collection")
    parser.add_argument("--run", type=Path, help="run a test executable and generate a report in one step")
    parser.add_argument("--output-dir", type=Path, help="new collection directory for --run (default: unique directory)")
    parser.add_argument("--source-root", type=Path, default=Path.cwd(), help="default source scope for --run")
    parser.add_argument("--include", help="regex selecting production source paths (forward slashes)")
    parser.add_argument("--exclude", action="append", default=[], help="additional source path exclusion regex")
    parser.add_argument("--llvm-profdata", default="llvm-profdata")
    parser.add_argument("--llvm-cov", default="llvm-cov")
    parser.add_argument("--output", type=Path, help="JSON output (default: collection/report.json in --run mode)")
    argv = sys.argv[1:]
    separator = argv.index("--") if "--" in argv else len(argv)
    args = parser.parse_args(argv[:separator])
    test_args = argv[separator + 1:]
    test_status = 0
    automatic_scope = args.include is None
    # Validate scope and dependencies before spending time running the suite.
    if args.include is not None:
        re.compile(args.include)
    for pattern in args.exclude:
        re.compile(pattern)
    if args.run:
        if args.directory or args.binary:
            parser.error("--run cannot be combined with offline directory/--binary")
        if any(a in ("-c", "--coverage") or a.startswith("--coverage-output=") for a in test_args):
            parser.error("pass collection options before --; recursive --coverage is not allowed")
        for tool in (args.llvm_profdata, args.llvm_cov):
            if not shutil.which(tool):
                raise ValueError(f"{tool} not found; install the LLVM coverage tools matching your Clang")
            run([tool, "--version"])
        executable = str(args.run) if args.run.is_file() else (shutil.which(str(args.run)) or str(args.run))
        args.binary = Path(executable).resolve(strict=True)
        args.directory = args.output_dir or (Path.cwd() / "zeroerr-coverage" / ("run-" + uuid.uuid4().hex))
        if args.directory.exists():
            raise ValueError(f"output directory already exists: {args.directory}")
        args.output = args.output or args.directory / "report.json"
        print(f"Coverage collection: {args.directory}", flush=True)
        test_status = subprocess.run([str(args.binary), *test_args,
                                     "--coverage-output=" + str(args.directory)]).returncode
        if "--list-test-cases" in test_args or "-l" in test_args:
            return test_status
        if test_status not in (0, 1):
            raise RuntimeError(f"test process did not complete (exit {test_status}); no report generated")
        if args.include is None:
            args.include = "^" + re.escape(args.source_root.resolve().as_posix().rstrip("/")) + r"(/|$)"
    elif not (args.directory and args.binary and args.include is not None and args.output):
        parser.error("offline mode requires directory, --binary, --include and --output")
    elif test_args or args.output_dir:
        parser.error("test arguments and --output-dir require --run")
    manifest = [json.loads(line) for line in (args.directory / "manifest.jsonl").read_text(encoding="utf-8").splitlines()]
    if not manifest or manifest[0] != {"schema": 1, "granularity": "test_case"}:
        raise ValueError("unsupported collection manifest")
    if manifest[-1] != {"complete": True}:
        raise ValueError("collection is incomplete; refusing partial statistics")
    rows = manifest[1:-1]
    if not rows:
        raise ValueError("no tests were collected")
    if automatic_scope:
        # Avoid calling two tests different merely because their own bodies differ.
        args.exclude.extend("^" + re.escape(row["file"].replace("\\", "/")) + "$" for row in rows)
        args.exclude.extend([r"/(third_party|_deps)/", r"/include/zeroerr/", r"/zeroerr\.hpp$"])
        framework = Path(__file__).resolve().parent.parent / "src"
        args.exclude.append("^" + re.escape(framework.as_posix()) + "/")
        args.exclude = list(dict.fromkeys(args.exclude))
    include, exclude = re.compile(args.include), [re.compile(p) for p in args.exclude]
    coverage, universe = [], None
    binary = str(args.binary.resolve(strict=True))
    with tempfile.TemporaryDirectory(prefix="zeroerr-coverage-") as tmp:
        indexed = str(Path(tmp) / "test.profdata")
        for row in rows:
            profile = row["profile"]
            if not re.fullmatch(r"test-[0-9]+\.profraw", profile):
                raise ValueError("invalid profile filename")
            run([args.llvm_profdata, "merge", "-sparse", str(args.directory / profile), "-o", indexed])
            exported = json.loads(run([args.llvm_cov, "export", binary, "-instr-profile=" + indexed]))
            hits, available = extract(exported, include, exclude)
            if universe is not None and universe != available:
                raise ValueError("coverage feature universe changed between tests")
            universe = available
            coverage.append(hits)
    if not any(universe.values()):
        raise ValueError("source scope matched no instrumented code; check --include and build flags. "
                         "Tests sharing a file with production code need an explicit --include regex.")
    report = analyze(rows, coverage, universe)
    report["build"] = {"binary": binary, "sha256": hashlib.sha256(Path(binary).read_bytes()).hexdigest()}
    report["scope"] = {"include": args.include, "exclude": args.exclude}
    args.output.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    summary = [f"Tests: {len(rows)}; identical-coverage candidate groups: {len(report['identical_coverage_candidates'])}"]
    for metric, stats in report["summary"].items():
        percent = f" ({100 * stats['covered'] / stats['total']:.1f}%)" if stats['total'] else " (no mapped features)"
        summary.append(f"{metric}: {stats['covered']}/{stats['total']}{percent}")
    summary.append("\nSlowest tests (instrumented seconds):")
    for test in sorted(report["tests"], key=lambda t: t["seconds"], reverse=True)[:10]:
        summary.append(f"  {test['seconds']:.6f}s  {test['name']}")
    summary.append("\nIdentical measured coverage candidates (not proof of redundant assertions):")
    groups = report["identical_coverage_candidates"]
    for group in groups[:10]:
        summary.append("  " + " | ".join(report["tests"][i]["name"] for i in group))
    if not groups:
        summary.append("  None")
    if len(groups) > 10:
        summary.append("  Remaining groups are in the JSON report.")
    summary_path = args.output.with_suffix(".summary.txt")
    summary_path.write_text("\n".join(summary) + "\n", encoding="utf-8")
    print("\n".join(summary))
    print(f"Report: {args.output}")
    print(f"Summary: {summary_path}")
    return test_status


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, ValueError, RuntimeError, KeyError) as error:
        print(f"Coverage report: {error}", file=sys.stderr)
        sys.exit(2)
