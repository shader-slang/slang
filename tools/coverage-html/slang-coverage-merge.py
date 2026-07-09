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
- Each `--strip-prefix` (repeatable) is removed from the start of
  every record path. No project-specific prefixes are baked in;
  callers that want runner-root stripping inject the relevant
  prefixes themselves (the `tools/coverage/slang-merge.py`
  wrapper does this for the three Slang CI runners).

Standard library only; no pip deps.
"""

import argparse
import atexit
import gzip
import io
import os
import re
import sys
from typing import Dict, List, Optional, Tuple

# Shared parser / writer / data model.
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from lcov_io import (  # noqa: E402
    AuthFileSummary,
    AuthSummary,
    FileRecord,
    Function,
    LcovParseError,
    merge_auth_summaries,
    parse_lcov,
    parse_llvm_cov_report,
    write_lcov,
    write_llvm_cov_report,
)
from llvm_cov_json import is_json_input  # noqa: E402

GENERATOR_NAME = "slang-coverage-merge"


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


_temp_files: List[str] = []


@atexit.register
def _cleanup_temp_files() -> None:
    """Best-effort cleanup of gunzipped temp files on process exit.

    The merger may decompress multiple `.gz` inputs into temp files;
    on long-lived hosts (e.g. CI runners with persistent /tmp), they
    accumulate without this hook. `os.remove` may race or fail if
    the file is already gone — swallow any error.
    """
    for p in _temp_files:
        try:
            os.remove(p)
        except OSError:
            pass


def _gunzip_to_temp(gz_path: str, suffix: str = ".lcov") -> str:
    """Decompress a .gz file to a temp file and return the temp path.

    `parse_lcov` and `parse_llvm_cov_report` both take a path, not a
    stream, so we materialize the decompressed text on disk. The
    temp file is registered for cleanup via `atexit` (see
    `_cleanup_temp_files`).

    No upper bound on decompressed size — inputs are expected to be
    trusted CI artifacts. A crafted gzip bomb could exhaust /tmp;
    callers passing untrusted inputs should set their own size cap
    before invoking this tool.
    """
    import tempfile

    fd, tmp = tempfile.mkstemp(suffix=suffix, prefix="scv-merge-")
    _temp_files.append(tmp)
    with os.fdopen(fd, "wb") as out, gzip.open(gz_path, "rb") as gz:
        while True:
            chunk = gz.read(1 << 16)
            if not chunk:
                break
            out.write(chunk)
    return tmp


def load(path: str) -> List[FileRecord]:
    """Parse a possibly-gzipped LCOV file into FileRecords.

    JSON inputs (`llvm-cov export -format=json`) are rejected: the
    merger collapses to LCOV records and gzipped LCOV files are the
    only shape that survives the existing aggregation rules. To merge
    a JSON export across OSes, render each per-OS JSON to its own HTML
    and merge at the LCOV layer separately, or feed an LCOV converted
    from the JSON in upstream.
    """
    if is_json_input(path):
        # Match the renderer's exit-2 contract for "wrong kind of
        # input file" errors so CI / scripting callers can grep on
        # a single code across both tools.
        print(
            f"slang-coverage-merge: {path}: JSON coverage exports are "
            f"not supported as merger input. Pass the LCOV variant of "
            f"this artifact instead.",
            file=sys.stderr,
        )
        raise SystemExit(2)
    if path.lower().endswith(".gz"):
        path = _gunzip_to_temp(path)
    return parse_lcov(path, warn_prefix=GENERATOR_NAME)


def load_auth_summary(path: str) -> AuthSummary:
    """Parse a possibly-gzipped `llvm-cov report` text dump."""
    if path.lower().endswith(".gz"):
        path = _gunzip_to_temp(path, suffix=".txt")
    return parse_llvm_cov_report(path)


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
        # NOTE: must distinguish "absent" from "previously zero".
        # Using `dict.get(ln, 0)` would treat both as the same and
        # silently drop a 0-hit DA record present only in `src`.
        if ln not in dst.lines or hits > dst.lines[ln]:
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
            "Path prefix to strip from SF: paths (repeatable). "
            "Backslashes inside the prefix are normalized to forward "
            "slashes before matching. The longest matching prefix "
            "wins. Use this to collapse per-OS CI runner roots into "
            "a stable repo-relative path."
        ),
    )
    p.add_argument(
        "--filter-include-regex",
        action="append",
        default=[],
        metavar="REGEX",
        help=(
            "Include-only Python regex (repeatable; matched anywhere "
            "in the SF: path). Applied after path normalization."
        ),
    )
    p.add_argument(
        "--filter-exclude-regex",
        action="append",
        default=[],
        metavar="REGEX",
        help=(
            "Exclude Python regex (repeatable; matched anywhere in "
            "the SF: path). Applied after path normalization, before "
            "merging — so it always sees repo-relative paths "
            "regardless of input shape."
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
        "--auth-summary",
        action="append",
        default=[],
        metavar="REPORT_TXT",
        help=(
            "Path to an `llvm-cov report` text dump. Repeat once per "
            "input LCOV — order doesn't have to match. Each input is "
            "merged into one combined summary by max(total) / "
            "min(missed) per file. Use with --auth-summary-out to "
            "write the merged summary to disk for the renderer's "
            "--auth-summary flag to consume. Files dropped by "
            "--filter-exclude-regex / --filter-include-regex are "
            "dropped from the merged summary too."
        ),
    )
    p.add_argument(
        "--auth-summary-out",
        default=None,
        metavar="PATH",
        help=(
            "Output path for the merged `llvm-cov report` text "
            "synthesized from --auth-summary inputs. Required when "
            "--auth-summary is used."
        ),
    )
    p.add_argument(
        "--quiet",
        action="store_true",
        help="Suppress progress output on stderr.",
    )
    return p


def _filter_records(
    recs: List[FileRecord],
    inc_re: Optional["re.Pattern"],
    exc_re: Optional["re.Pattern"],
) -> List[FileRecord]:
    if not inc_re and not exc_re:
        return recs
    out: List[FileRecord] = []
    for r in recs:
        if inc_re and not inc_re.search(r.path):
            continue
        if exc_re and exc_re.search(r.path):
            continue
        out.append(r)
    return out


def main(argv: Optional[List[str]] = None) -> int:
    args = build_argparser().parse_args(argv)

    prefixes: List[str] = list(args.strip_prefix)

    try:
        inc_re = (
            re.compile("|".join(args.filter_include_regex))
            if args.filter_include_regex
            else None
        )
        exc_re = (
            re.compile("|".join(args.filter_exclude_regex))
            if args.filter_exclude_regex
            else None
        )
    except re.error as e:
        print(
            f"{GENERATOR_NAME}: invalid --filter-include-regex / "
            f"--filter-exclude-regex: {e}",
            file=sys.stderr,
        )
        return 2

    inputs: List[List[FileRecord]] = []
    for path in args.inputs:
        try:
            recs = load(path)
        except LcovParseError as e:
            print(f"{GENERATOR_NAME}: {e}", file=sys.stderr)
            return 2
        recs = normalized_records(recs, tuple(prefixes))
        before = len(recs)
        recs = _filter_records(recs, inc_re, exc_re)
        inputs.append(recs)
        if not args.quiet:
            line = (
                f"{GENERATOR_NAME}: {path}: {len(recs)} files / "
                f"{sum(r.total_lines for r in recs)} lines / "
                f"{sum(r.total_branches for r in recs)} branches / "
                f"{sum(r.total_functions for r in recs)} functions"
            )
            if before != len(recs):
                line += f" (filter dropped {before - len(recs)})"
            print(line, file=sys.stderr)

    # Merge per-OS llvm-cov report dumps into one if requested. Done
    # in a separate pass so it stays independent of LCOV merging
    # — paths in the reports are already repo-relative when CI dumps
    # them, so no normalization is required here.
    if args.auth_summary:
        if not args.auth_summary_out:
            print(
                f"{GENERATOR_NAME}: error: --auth-summary-out is "
                f"required when --auth-summary is given",
                file=sys.stderr,
            )
            return 2
        per_os: List[AuthSummary] = []
        for p in args.auth_summary:
            try:
                per_os.append(load_auth_summary(p))
            except LcovParseError as e:
                print(f"{GENERATOR_NAME}: {e}", file=sys.stderr)
                return 2
        merged_auth = merge_auth_summaries(per_os)
        # Apply the same include/exclude regex filters to the auth
        # summary so the merged report matches the LCOV record set.
        dropped = 0
        if inc_re or exc_re:
            before = len(merged_auth.files)
            merged_auth.files = {
                k: v
                for k, v in merged_auth.files.items()
                if (not inc_re or inc_re.search(k))
                and (not exc_re or not exc_re.search(k))
            }
            dropped = before - len(merged_auth.files)
        # Recompute `merged_auth.total` from the per-file rows so
        # the TOTAL row and the stderr summary always match
        # `sum(merged_auth.files.values())`. `merge_auth_summaries`
        # reduces per-OS totals via max/min for cross-OS robustness;
        # for the merged-output TOTAL, the cross-file sum of the
        # already-reduced per-file rows is the consistent answer.
        recomputed = AuthFileSummary()
        for fs in merged_auth.files.values():
            recomputed.line_total += fs.line_total
            recomputed.line_missed += fs.line_missed
            recomputed.func_total += fs.func_total
            recomputed.func_missed += fs.func_missed
            recomputed.branch_total += fs.branch_total
            recomputed.branch_missed += fs.branch_missed
        merged_auth.total = recomputed
        if dropped and not args.quiet:
            print(
                f"{GENERATOR_NAME}: filter dropped {dropped} "
                f"entr{'y' if dropped == 1 else 'ies'} from auth-summary",
                file=sys.stderr,
            )
        with open(args.auth_summary_out, "w", encoding="utf-8") as f:
            write_llvm_cov_report(merged_auth, f)
        if not args.quiet:
            print(
                f"{GENERATOR_NAME}: merged {len(args.auth_summary)} "
                f"auth-summary input(s) into {args.auth_summary_out}: "
                f"{len(merged_auth.files)} files / "
                f"lines {merged_auth.total.line_hit}/{merged_auth.total.line_total} / "
                f"functions {merged_auth.total.func_hit}/{merged_auth.total.func_total} / "
                f"branches {merged_auth.total.branch_hit}/{merged_auth.total.branch_total}",
                file=sys.stderr,
            )

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
