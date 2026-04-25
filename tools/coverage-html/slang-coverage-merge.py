#!/usr/bin/env python3
"""
slang-coverage-merge — merge N LCOV `.info` files into one.

Companion to `slang-coverage-html`. Reads multiple LCOV inputs
(typically one per CI host: linux, macos, windows), normalizes their
SF: paths to repo-relative form, max-aggregates per-line / per-branch
/ per-function counts across files, and writes a single combined
LCOV to stdout or a path. Pipe the result to the renderer:

    slang-coverage-merge linux.lcov macos.lcov windows.lcov \\
        | slang-coverage-html /dev/stdin --output-dir merged-html/

Aggregation rules (max-across-inputs):
- DA hit counts: max.
- BRDA taken counts: max; an integer beats `-` (None); "absent on
  this input" is treated as "no information", not "0".
- FN/FNDA: first-FN line wins; max FNDA hit count.
- LF/LH/BRF/BRH/FNF/FNH are recomputed from merged data and the
  input-side values are ignored.
- TN: dropped from output (the merged file is not attributable to a
  single test name; per-test attribution is a separate feature).

Path normalization:
- Backslashes → forward slashes (Windows artifacts).
- Built-in default prefixes for the three Slang CI runners are
  stripped; `--strip-prefix` extends the list.

Standard library only; no pip deps.
"""

import argparse
import gzip
import io
import os
import sys
from typing import Dict, List, Optional, Tuple

# Shared parser / writer / data model.
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from lcov_io import (  # noqa: E402
    FileRecord,
    Function,
    LcovParseError,
    apply_slangc_filter,
    parse_lcov,
    write_lcov,
)

GENERATOR_NAME = "slang-coverage-merge"

# Path prefixes seen on Slang CI runners. Stripping them produces a
# repo-relative path that's stable across hosts. Backslashes are
# already normalized to forward slashes before this is applied.
DEFAULT_STRIP_PREFIXES: Tuple[str, ...] = (
    "/__w/slang/slang/",                  # Linux GitHub Actions
    "/Users/runner/work/slang/slang/",    # macOS GitHub Actions
    "D:/a/slang/slang/",                  # Windows GitHub Actions
)


# ---------------------------------------------------------------------------
# Path normalization
# ---------------------------------------------------------------------------


def normalize_path(raw: str, strip_prefixes: Tuple[str, ...]) -> str:
    """Repo-relative form: backslash → slash, then strip the longest
    matching prefix (if any). Pass through unchanged on no match."""
    p = raw.replace("\\", "/")
    # Sort by length desc so the longest prefix wins.
    for prefix in sorted(strip_prefixes, key=len, reverse=True):
        norm_prefix = prefix.replace("\\", "/")
        if p.startswith(norm_prefix):
            return p[len(norm_prefix):]
    return p


def normalized_records(
    records: List[FileRecord], strip_prefixes: Tuple[str, ...]
) -> List[FileRecord]:
    """Return records with `path` normalized in place."""
    for r in records:
        r.path = normalize_path(r.path, strip_prefixes)
    return records


# ---------------------------------------------------------------------------
# Input loading (incl. gzip auto-detect)
# ---------------------------------------------------------------------------


def _gunzip_to_temp(gz_path: str) -> str:
    """Decompress a .gz LCOV to a temp file and return the temp path.

    parse_lcov takes a path, not a stream — so we materialize the
    decompressed text on disk. The temp file is left in place; callers
    are CLI invocations with short lifetimes, OS will clean it up.
    """
    import tempfile

    fd, tmp = tempfile.mkstemp(suffix=".lcov", prefix="scv-merge-")
    with os.fdopen(fd, "wb") as out, gzip.open(gz_path, "rb") as gz:
        while True:
            chunk = gz.read(1 << 16)
            if not chunk:
                break
            out.write(chunk)
    return tmp


def load(path: str) -> List[FileRecord]:
    """Parse a possibly-gzipped LCOV file into FileRecords."""
    if path.endswith(".gz"):
        path = _gunzip_to_temp(path)
    return parse_lcov(path, warn_prefix=GENERATOR_NAME)


# ---------------------------------------------------------------------------
# Function synthesis (from sibling FN positions + own DA)
# ---------------------------------------------------------------------------


def synthesize_missing_functions(
    inputs: List[List[FileRecord]],
) -> int:
    """For each file present in multiple inputs, fill in FN/FNDA on
    inputs that lack them, using sibling inputs' (name, first_line)
    map and the silent input's own DA hit count at first_line as the
    proxy call count.

    Returns the number of synthesized FNDA entries.

    Use case: the OpenCppCoverage → LCOV converter on Windows emits
    DA records but no FN/FNDA. Linux/macOS (clang's llvm-cov export)
    emit both. Synthesis lets a 3-OS merge surface a per-function
    Windows hit count that rides off Linux/macOS's function map and
    Windows's own line-execution data.

    Modifies `inputs` in place.
    """
    # path -> {name: first_line}, deduped across inputs (first FN wins).
    fn_positions: Dict[str, Dict[str, int]] = {}
    for record_list in inputs:
        for r in record_list:
            for name, fn in r.functions.items():
                if fn.first_line <= 0:
                    continue
                fn_positions.setdefault(r.path, {}).setdefault(name, fn.first_line)

    synthesized = 0
    for record_list in inputs:
        for r in record_list:
            if r.functions:
                # This input already reported functions for this file;
                # leave it alone.
                continue
            positions = fn_positions.get(r.path)
            if not positions:
                # No sibling input has FN data for this file either.
                continue
            for name, first_line in positions.items():
                hits = r.lines.get(first_line, 0)
                r.functions[name] = Function(
                    first_line=first_line, hits=hits
                )
                synthesized += 1
    return synthesized


# ---------------------------------------------------------------------------
# Cross-file merge
# ---------------------------------------------------------------------------


def merge_records(
    inputs: List[List[FileRecord]],
) -> List[FileRecord]:
    """Combine per-input record lists into one by file path.

    Each `FileRecord.path` must already be in repo-relative form
    (i.e. caller has run normalize_path).
    """
    out: Dict[str, FileRecord] = {}
    for record_list in inputs:
        for r in record_list:
            existing = out.get(r.path)
            if existing is None:
                out[r.path] = r
                continue
            _merge_into(existing, r)
    # Stable order: alphabetical by path. Output order doesn't affect
    # correctness, but stable ordering helps tests and diffs.
    return [out[k] for k in sorted(out)]


def _merge_into(dst: FileRecord, src: FileRecord) -> None:
    """Fold `src`'s coverage into `dst` in place using max rules."""
    # Lines: max hit count per source line.
    for ln, hits in src.lines.items():
        prev = dst.lines.get(ln, 0)
        if hits > prev:
            dst.lines[ln] = hits

    # Branches: integer beats None; integer max wins.
    for key, taken in src.branches.items():
        if key not in dst.branches:
            dst.branches[key] = taken
            continue
        prev = dst.branches[key]
        if taken is None:
            # `-` carries no information; keep whatever we had.
            continue
        if prev is None:
            dst.branches[key] = taken
        elif taken > prev:
            dst.branches[key] = taken

    # Functions: first FN line declaration wins; max hit count wins.
    for name, fn in src.functions.items():
        existing = dst.functions.get(name)
        if existing is None:
            dst.functions[name] = Function(
                first_line=fn.first_line, hits=fn.hits
            )
        else:
            if existing.first_line == 0 and fn.first_line > 0:
                existing.first_line = fn.first_line
            if fn.hits > existing.hits:
                existing.hits = fn.hits


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------


def build_argparser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        prog="slang-coverage-merge",
        description=(
            "Merge multiple LCOV .info files into one combined output."
        ),
    )
    p.add_argument(
        "inputs",
        nargs="+",
        metavar="LCOV",
        help="LCOV .info file(s) to merge. .gz inputs are auto-decompressed.",
    )
    p.add_argument(
        "-o",
        "--output",
        default="-",
        help='Output path (default: stdout, "-").',
    )
    p.add_argument(
        "--strip-prefix",
        action="append",
        default=[],
        metavar="PREFIX",
        help=(
            "Extra path prefix to strip from SF: paths (repeatable). "
            "Backslashes inside the prefix are normalized to forward "
            "slashes before matching. Built-in defaults already cover "
            "the three Slang CI runner roots; use this to add extras."
        ),
    )
    p.add_argument(
        "--no-default-prefixes",
        action="store_true",
        help=(
            "Skip the built-in path prefixes "
            f"({', '.join(DEFAULT_STRIP_PREFIXES)})."
        ),
    )
    p.add_argument(
        "--slangc-filter",
        action="store_true",
        help=(
            "Restrict the merged output to the slangc compiler-only file "
            "set CI uses (mirrors tools/coverage/slangc-ignore-patterns.sh: "
            "drops external/, build/prelude/, generated FIDDLE / capability "
            "tables, language-server / record-replay / glslang, etc.). "
            "Applied after path normalization, before merging — so it "
            "always sees repo-relative paths regardless of input shape."
        ),
    )
    p.add_argument(
        "--synthesize-functions",
        action="store_true",
        help=(
            "EXPERIMENTAL. For inputs that emit DA but no FN/FNDA "
            "(notably the Windows OpenCppCoverage → LCOV converter), "
            "fill in FN/FNDA from sibling inputs' (name, first_line) "
            "map plus this input's own DA hit count at first_line. "
            "Lets Windows contribute to per-function coverage in the "
            "merged report. Caveat: DA-at-first-line is a proxy for "
            "FNDA call count and can over-count when the entry line "
            "is shared with a callee or fall-through; treat the "
            "result as an upper bound on function coverage."
        ),
    )
    p.add_argument(
        "--quiet",
        action="store_true",
        help="Suppress progress output on stderr.",
    )
    return p


def main(argv: Optional[List[str]] = None) -> int:
    args = build_argparser().parse_args(argv)

    prefixes: List[str] = []
    if not args.no_default_prefixes:
        prefixes.extend(DEFAULT_STRIP_PREFIXES)
    prefixes.extend(args.strip_prefix)

    inputs: List[List[FileRecord]] = []
    for path in args.inputs:
        try:
            recs = load(path)
        except LcovParseError as e:
            print(f"{GENERATOR_NAME}: {e}", file=sys.stderr)
            return 2
        recs = normalized_records(recs, tuple(prefixes))
        before = len(recs)
        if args.slangc_filter:
            recs = apply_slangc_filter(recs)
        inputs.append(recs)
        if not args.quiet:
            line = (
                f"{GENERATOR_NAME}: {path}: {len(recs)} files / "
                f"{sum(r.total_lines for r in recs)} lines / "
                f"{sum(r.total_branches for r in recs)} branches / "
                f"{sum(r.total_functions for r in recs)} functions"
            )
            if args.slangc_filter:
                line += f" (slangc-filter dropped {before - len(recs)})"
            print(line, file=sys.stderr)

    if args.synthesize_functions:
        synth = synthesize_missing_functions(inputs)
        if not args.quiet and synth > 0:
            print(
                f"{GENERATOR_NAME}: --synthesize-functions: filled in "
                f"{synth} FNDA entr{'y' if synth == 1 else 'ies'} for "
                f"inputs lacking FN/FNDA (using sibling FN positions "
                f"+ own DA hits at first_line; treat as upper bound)",
                file=sys.stderr,
            )

    merged = merge_records(inputs)

    if args.output == "-":
        write_lcov(merged, sys.stdout)
    else:
        with open(args.output, "w", encoding="utf-8") as f:
            write_lcov(merged, f)

    if not args.quiet:
        total_l = sum(r.total_lines for r in merged)
        hit_l = sum(r.hit_lines for r in merged)
        total_br = sum(r.total_branches for r in merged)
        hit_br = sum(r.hit_branches for r in merged)
        total_fn = sum(r.total_functions for r in merged)
        hit_fn = sum(r.hit_functions for r in merged)
        out_label = "stdout" if args.output == "-" else args.output
        msg = (
            f"{GENERATOR_NAME}: merged {len(args.inputs)} input(s) into "
            f"{out_label}: {len(merged)} files / "
            f"lines {hit_l}/{total_l} ("
            f"{(100.0 * hit_l / total_l) if total_l else 0.0:.1f}%)"
        )
        if total_br:
            msg += f" / branches {hit_br}/{total_br} ({100.0 * hit_br / total_br:.1f}%)"
        if total_fn:
            msg += f" / functions {hit_fn}/{total_fn} ({100.0 * hit_fn / total_fn:.1f}%)"
        print(msg, file=sys.stderr)

    return 0


if __name__ == "__main__":
    sys.exit(main())
