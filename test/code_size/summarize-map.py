"""Summarize this probe's MSVC /MAP output (symbol spans include padding).

Usage: python test/code_size/summarize-map.py BUILD_DIR/Release/code_size_probe.map
"""
from collections import Counter, defaultdict
import json
from pathlib import Path
import re
import sys

lines = Path(sys.argv[1]).read_text().splitlines()
entries = defaultdict(list)
for line in lines:
    match = re.match(
        r" 0001:([0-9a-f]+)\s+(\S+)\s+[0-9a-f]+\s+f\s+(?:i\s+)?(\S+)", line
    )
    if match:
        entries[int(match[1], 16)].append((match[2], match[3]))
addresses = sorted(entries)
spans = Counter()
exercises = []
for start, end in zip(addresses, addresses[1:]):
    symbol, obj = entries[start][0]
    spans[obj] += end - start
    if symbol.startswith("??$exercise@"):
        exercises.append(end - start)


def family(prefix):
    matches = [address for address, symbols in entries.items()
               for name, obj in symbols if name.startswith(prefix)]
    return {"symbols": len(matches), "unique_addresses": len(set(matches))}


result = {
    "main_public_symbol_spans": spans["main.obj"],
    "text_public_symbol_spans": spans["zeroerr:text.obj"],
    "color_object_linked": any("zeroerr:color.obj" in line for line in lines),
    "exercise_function_count": len(exercises),
    "exercise_span_min": min(exercises),
    "exercise_span_max": max(exercises),
    "format": family("??$format@U?$Value"),
    "comparison": family("??$?8H@?$Expression@AEAU?$Value"),
    "callback": family("??$printDiagnosticValue@"),
}
print(json.dumps(result, indent=2))
