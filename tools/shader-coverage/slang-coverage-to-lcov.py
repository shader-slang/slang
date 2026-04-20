#!/usr/bin/env python3
"""
Convert a Slang coverage counter buffer + manifest to LCOV .info format.

Pipeline:
    1. Compile a shader with:
           slangc shader.slang -fcoverage ...
       with SLANG_COVERAGE_MANIFEST_PATH=shader.slangcov set in the
       environment.  This produces an instrumented shader plus a JSON
       sidecar describing each counter's (file, line).
    2. Dispatch the shader on your runtime of choice, bound to the
       buffer whose location is recorded in the manifest's `buffer`
       block (register/space or binding/descriptor_set).
    3. Read the UAV back to a binary file of little-endian uint32's,
       one per counter.
    4. Run this script:
           slang-coverage-to-lcov.py \\
               --manifest shader.slangcov \\
               --counters shader.buffer.bin \\
               --output shader.lcov

Supported counter input formats:
    --counters <file>          Binary: uint32 little-endian * N (default)
    --counters-text <file>     Text: whitespace-separated decimal ints
    --counters-text -          Text from stdin (handy for piping
                               slang-test output through `grep | awk`)

The output is standard LCOV .info.  Feed it into:
    genhtml shader.lcov -o html/
    # or upload to Codecov, consume in VS Code Coverage Gutters, etc.
"""

import argparse
import collections
import json
import struct
import sys


def load_counters_binary(path, count):
    with open(path, "rb") as f:
        raw = f.read()
    if len(raw) < count * 4:
        sys.exit(
            f"error: counter file '{path}' is {len(raw)} bytes; "
            f"manifest expects at least {count * 4}"
        )
    return list(struct.unpack(f"<{count}I", raw[: count * 4]))


def load_counters_text(src, count):
    data = sys.stdin.read() if src == "-" else open(src).read()
    values = [int(tok) for tok in data.split() if tok.strip()]
    if len(values) < count:
        sys.exit(
            f"error: found {len(values)} counter values; "
            f"manifest expects at least {count}"
        )
    return values[:count]


def main():
    p = argparse.ArgumentParser(
        description="Convert Slang coverage buffer + manifest to LCOV .info."
    )
    p.add_argument("--manifest", required=True, help="path to .slangcov JSON")
    p.add_argument("--counters", help="binary uint32 little-endian counter buffer")
    p.add_argument(
        "--counters-text", help="whitespace-separated integers (file or '-' for stdin)"
    )
    p.add_argument("--output", default="-", help="output .lcov path (default: stdout)")
    p.add_argument(
        "--test-name",
        default="slang_coverage",
        help="test-name (TN:) field in LCOV output. LCOV forbids "
        "hyphens in test names — use underscores or alphanumerics.",
    )
    args = p.parse_args()

    if (args.counters is None) == (args.counters_text is None):
        sys.exit("error: exactly one of --counters or --counters-text is required")

    with open(args.manifest) as f:
        manifest = json.load(f)

    version = manifest.get("version", 1)
    if version != 1:
        sys.exit(f"error: unsupported manifest version {version}")

    total = int(manifest["counters"])
    if args.counters:
        counters = load_counters_binary(args.counters, total)
    else:
        counters = load_counters_text(args.counters_text, total)

    # Aggregate by (file, line).  The pass already dedupes at
    # compile time, so normally each (file, line) maps to a single
    # counter — but accept multiple mappings gracefully by summing.
    hits_by_line = collections.defaultdict(lambda: collections.defaultdict(int))
    for entry in manifest["entries"]:
        idx = entry["index"]
        if idx >= len(counters):
            sys.exit(f"error: entry index {idx} exceeds counter buffer size")
        hits_by_line[entry["file"]][int(entry["line"])] += counters[idx]

    out = sys.stdout if args.output == "-" else open(args.output, "w")
    out.write(f"TN:{args.test_name}\n")
    for source in sorted(hits_by_line):
        out.write(f"SF:{source}\n")
        for line in sorted(hits_by_line[source]):
            out.write(f"DA:{line},{hits_by_line[source][line]}\n")
        out.write("end_of_record\n")
    if out is not sys.stdout:
        out.close()


if __name__ == "__main__":
    main()
