#!/usr/bin/env python3
"""
Convert a Slang coverage counter buffer + manifest to LCOV .info format.

Pipeline:
    1. Compile a shader with:
           slangc shader.slang -trace-coverage ...
       This instruments the shader with a synthesized
       `RWStructuredBuffer<uint64_t> __slang_coverage` by default, or
       `RWStructuredBuffer<uint>` when
       `-trace-coverage-counter-width 32` is passed (e.g. for
       runtime drivers that lack 64-bit shader atomic add).
       Counter slots are assigned according to the coverage metadata.
       The current line/function/branch modes use one direct counter
       per marker op, but consumers must treat the manifest as
       authoritative.
    2. Read the `.coverage-mapping.json` sidecar describing each
       source coverage entry's counter/source mapping, or query the
       same data through
       `ICoverageTracingMetadata`.
    3. Dispatch the shader, bound to the coverage buffer at the
       reflected binding reported by the metadata / sidecar.
    4. Read the UAV back to a binary file of little-endian unsigned
       integers — one per counter, with the byte width given by
       `manifest.buffer.element_stride` (4 for uint32, 8 for uint64).
    5. Run this script:
           slang-coverage-to-lcov.py \\
               --manifest shader.spv.coverage-mapping.json \\
               --counters shader.buffer.bin \\
               --output shader.lcov

Supported counter input formats:
    --counters <file>          Binary: little-endian unsigned ints of
                               the width reported in the manifest's
                               `buffer.element_stride` (uint32 or
                               uint64). Width is auto-detected from
                               the manifest; no flag needed.
    --counters-text <file>     Text: whitespace-separated decimal ints
                               (width-agnostic).
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


# Map manifest `element_stride` to the `struct` module's little-endian
# format character. The IR coverage pass synthesizes either a
# `RWStructuredBuffer<uint>` (4 bytes / "I") or a
# `RWStructuredBuffer<uint64_t>` (8 bytes / "Q") depending on
# `-trace-coverage-counter-width`; both produce host buffers of
# consecutive little-endian unsigned integers of the matching width.
_STRIDE_TO_STRUCT_CODE = {4: "I", 8: "Q"}


def load_counters_binary(path, count, stride):
    code = _STRIDE_TO_STRUCT_CODE.get(stride)
    if code is None:
        sys.exit(
            f"error: unsupported element_stride {stride} from manifest; "
            f"slang-coverage-to-lcov.py only handles 4 (uint32) and 8 (uint64)"
        )
    needed = count * stride
    with open(path, "rb") as f:
        raw = f.read()
    if len(raw) < needed:
        sys.exit(
            f"error: counter file '{path}' is {len(raw)} bytes; "
            f"manifest expects at least {needed} ({count} slots * {stride} bytes)"
        )
    return list(struct.unpack(f"<{count}{code}", raw[:needed]))


def load_counters_text(src, count):
    data = sys.stdin.read() if src == "-" else open(src).read()
    values = [int(tok) for tok in data.split() if tok.strip()]
    if len(values) < count:
        sys.exit(
            f"error: found {len(values)} counter values; "
            f"manifest expects at least {count}"
        )
    return values[:count]


def get_manifest_version(manifest):
    version = manifest.get("version", 1)
    if version not in (1, 2):
        sys.exit(f"error: unsupported manifest version {version}")
    return version


def parse_manifest_int(value, field_name):
    if isinstance(value, bool):
        sys.exit(f"error: manifest {field_name} must be an integer")
    if isinstance(value, int):
        return value
    if isinstance(value, str):
        text = value.strip()
        if text and text.lstrip("-").isdigit():
            return int(text)
    sys.exit(f"error: manifest {field_name} must be an integer")


def get_manifest_counter_count(manifest):
    version = get_manifest_version(manifest)
    try:
        if version == 1:
            total = parse_manifest_int(manifest["counters"], "counter count")
        else:
            total = parse_manifest_int(manifest["counter_count"], "counter count")
    except (KeyError, TypeError, ValueError):
        sys.exit("error: manifest is missing a valid counter count")
    if total < 0:
        sys.exit(f"error: manifest counter count must be non-negative, got {total}")
    return total


def get_manifest_element_stride(manifest):
    # Pull the per-slot byte width from `buffer.element_stride`.
    # Compilers that predate the
    # `-trace-coverage-counter-width` flag did not populate this
    # field; treat its absence as the historical uint32 layout so
    # this script can still read counters from those older outputs
    # without forcing a re-compile.
    buffer = manifest.get("buffer")
    if not isinstance(buffer, dict):
        return 4
    stride = buffer.get("element_stride")
    if stride is None:
        return 4
    parsed = parse_manifest_int(stride, "buffer element_stride")
    if parsed <= 0:
        sys.exit(
            f"error: manifest buffer.element_stride must be positive, got {parsed}"
        )
    return parsed


def iter_manifest_entries(manifest):
    version = get_manifest_version(manifest)
    try:
        entries = manifest["entries"]
    except KeyError:
        sys.exit("error: manifest is missing 'entries'")
    if not isinstance(entries, list):
        sys.exit("error: manifest entries must be an array")
    for entry in entries:
        if version == 1:
            try:
                yield {
                    "kind": "line",
                    "counter": parse_manifest_int(entry["index"], "v1 entry index"),
                    "file": entry.get("file"),
                    "line": parse_manifest_int(entry["line"], "v1 entry line"),
                }
            except (AttributeError, KeyError, TypeError, ValueError):
                sys.exit("error: invalid v1 entry in manifest")
            continue
        if version == 2:
            try:
                kind = entry.get("kind", "line")
                if not isinstance(kind, str):
                    sys.exit("error: manifest v2 entry kind must be a string")
                counter = entry.get("counter")
                yield {
                    "kind": kind,
                    "counter": None
                    if counter is None
                    else parse_manifest_int(counter, "v2 entry counter"),
                    "file": entry.get("file"),
                    "line": parse_manifest_int(entry["line"], "v2 entry line"),
                    "function": entry.get("function"),
                    "function_mangled": entry.get("function_mangled"),
                    "branch_site": entry.get("branch_site"),
                    "branch_arm": entry.get("branch_arm"),
                }
            except (AttributeError, KeyError, TypeError, ValueError):
                sys.exit("error: invalid v2 entry in manifest")
            continue


def main():
    p = argparse.ArgumentParser(
        description="Convert Slang coverage buffer + manifest to LCOV .info."
    )
    p.add_argument(
        "--manifest",
        required=True,
        help="path to .coverage-mapping.json (or equivalent JSON built from "
        "ICoverageTracingMetadata)",
    )
    p.add_argument(
        "--counters",
        help="binary counter buffer. Each slot is a little-endian unsigned "
        "integer of the width reported in the manifest's "
        "`buffer.element_stride` (4 for uint32, 8 for uint64).",
    )
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

    total = get_manifest_counter_count(manifest)
    stride = get_manifest_element_stride(manifest)
    if args.counters:
        counters = load_counters_binary(args.counters, total, stride)
    else:
        # Text mode is width-agnostic: the values are already decimal
        # ints, so the manifest stride is informational only.
        counters = load_counters_text(args.counters_text, total)

    # Aggregate by (file, line). Multiple counter slots may map to the
    # same line because the compiler assigns one slot per counter op;
    # LCOV wants line-oriented reporting, so sum them here.
    #
    # GCOV/LCOV-style output only admits real source files and positive
    # line numbers. Keep unattributable slots in the manifest/metadata,
    # but filter them out when exporting LCOV.
    hits_by_line = collections.defaultdict(lambda: collections.defaultdict(int))
    functions_by_source = collections.defaultdict(dict)
    branches_by_source = collections.defaultdict(dict)
    skipped_entries = 0
    skipped_by_kind = {}
    for entry in iter_manifest_entries(manifest):
        kind = entry["kind"]
        idx = entry["counter"]
        if idx is None:
            skipped_by_kind[f"{kind}-counterless"] = (
                skipped_by_kind.get(f"{kind}-counterless", 0) + 1
            )
            continue
        if idx < 0 or idx >= len(counters):
            sys.exit(f"error: counter index {idx} out of range [0, {len(counters)})")
        source = entry["file"]
        line = entry["line"]
        if not source or line <= 0:
            skipped_entries += 1
            continue
        count = counters[idx]

        if kind == "line":
            hits_by_line[source][line] += count
        elif kind == "function":
            function_name = entry.get("function") or entry.get("function_mangled")
            if not function_name:
                skipped_by_kind["function-without-name"] = (
                    skipped_by_kind.get("function-without-name", 0) + 1
                )
                continue
            old_line, old_count = functions_by_source[source].get(function_name, (line, 0))
            functions_by_source[source][function_name] = (min(old_line, line), old_count + count)
        elif kind == "branch":
            branch_site = entry.get("branch_site")
            branch_arm = entry.get("branch_arm")
            if branch_site is None or branch_arm is None:
                skipped_by_kind["branch-without-site"] = (
                    skipped_by_kind.get("branch-without-site", 0) + 1
                )
                continue
            branch_site = parse_manifest_int(branch_site, "v2 entry branch_site")
            branch_arm = parse_manifest_int(branch_arm, "v2 entry branch_arm")
            if branch_site < 0 or branch_arm < 0:
                sys.exit(
                    "error: manifest v2 entry branch_site and branch_arm "
                    "must be non-negative"
                )
            branch_key = (line, branch_site, branch_arm)
            branches_by_source[source][branch_key] = (
                branches_by_source[source].get(branch_key, 0) + count
            )
        else:
            skipped_by_kind[kind] = skipped_by_kind.get(kind, 0) + 1

    out = sys.stdout if args.output == "-" else open(args.output, "w")
    out.write(f"TN:{args.test_name}\n")
    sources = set(hits_by_line) | set(functions_by_source) | set(branches_by_source)
    for source in sorted(sources):
        out.write(f"SF:{source}\n")
        functions = functions_by_source[source]
        function_sort_key = lambda item: (item[1][0], item[0])
        for function_name, (line, _) in sorted(functions.items(), key=function_sort_key):
            out.write(f"FN:{line},{function_name}\n")
        for function_name, (_, count) in sorted(functions.items(), key=function_sort_key):
            out.write(f"FNDA:{count},{function_name}\n")
        if functions:
            out.write(f"FNF:{len(functions)}\n")
            out.write(f"FNH:{sum(1 for _, count in functions.values() if count > 0)}\n")
        branches = branches_by_source[source]
        for (line, branch_site, branch_arm), count in sorted(branches.items()):
            out.write(f"BRDA:{line},{branch_site},{branch_arm},{count}\n")
        if branches:
            out.write(f"BRF:{len(branches)}\n")
            out.write(f"BRH:{sum(1 for count in branches.values() if count > 0)}\n")
        for line in sorted(hits_by_line[source]):
            out.write(f"DA:{line},{hits_by_line[source][line]}\n")
        out.write("end_of_record\n")
    if out is not sys.stdout:
        out.close()
    if skipped_entries:
        print(
            f"note: skipped {skipped_entries} coverage entr"
            f"{'y' if skipped_entries == 1 else 'ies'} without attributable source location",
            file=sys.stderr,
        )
    for kind in sorted(skipped_by_kind):
        count = skipped_by_kind[kind]
        if kind.endswith("-counterless"):
            label = f"{kind[:-12]} entries without a runtime counter"
        elif kind == "function-without-name":
            label = "function entries without a name"
        elif kind == "branch-without-site":
            label = "branch entries without site/arm ids"
        else:
            label = f"entries of kind {kind!r}"
        print(
            f"note: skipped {count} {label} not representable in LCOV",
            file=sys.stderr,
        )


if __name__ == "__main__":
    main()
