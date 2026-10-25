"""Measure MSVC build artifacts without conflating debug data with machine code.

Usage: python test/code_size/measure.py BUILD_DIR Release [code_size_probe|unittest]
Build both revisions with the same compiler, flags, configuration and workload.
"""
import json
from pathlib import Path
import struct
import sys


def text_size(path):
    data = path.read_bytes()
    if data[:2] == b"MZ":
        coff = struct.unpack_from("<I", data, 0x3C)[0] + 4
        is_pe = True
    else:
        coff = 0
        is_pe = False
    _, sections, _, _, _, optional, _ = struct.unpack_from("<HHIIIHH", data, coff)
    start = coff + 20 + optional
    total = 0
    for i in range(sections):
        section = start + 40 * i
        name = data[section:section + 8].rstrip(b"\0")
        if name.startswith(b".text"):
            # PE virtual size excludes file alignment (Debug may still contain
            # incremental-link padding). COFF uses SizeOfRawData instead.
            total += struct.unpack_from("<I", data, section + (8 if is_pe else 16))[0]
    return total


root = Path(sys.argv[1])
config = sys.argv[2]
target = sys.argv[3] if len(sys.argv) > 3 else "code_size_probe"
executable, = root.glob(f"**/{config}/{target}.exe")
objects = list(root.glob(f"**/{target}.dir/{config}/*.obj"))
library_objects = list(root.glob(f"**/zeroerr.dir/{config}/*.obj"))
result = {
    "configuration": config,
    "target": target,
    "executable_bytes": executable.stat().st_size,
    "executable_text_bytes": text_size(executable),
    "target_object_bytes": sum(p.stat().st_size for p in objects),
    "target_object_text_bytes": sum(text_size(p) for p in objects),
    "library_object_bytes": sum(p.stat().st_size for p in library_objects),
    "library_object_text_bytes": sum(text_size(p) for p in library_objects),
}
pdb = executable.with_suffix(".pdb")
if pdb.exists():
    result["pdb_bytes"] = pdb.stat().st_size
print(json.dumps(result, indent=2))
