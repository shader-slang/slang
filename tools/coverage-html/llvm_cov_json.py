"""
Parser for `llvm-cov export -format=json` coverage exports.

Companion to `lcov_io.parse_lcov`: produces the same `FileRecord`
shape (lines / branches / functions dicts plus a regions list and an
authoritative auth_override) so the renderer can treat both inputs
uniformly.

Why a separate parser exists at all: LCOV is lossy on the LLVM path.
It has no record type for regions, BRDA over-emits relative to
`llvm-cov report`'s branch count, and FNDA undercounts inlined
functions. The JSON export is the same `.profdata` rendered without
that loss — branches/functions/regions all come straight from
LLVM's coverage mapping, and the per-file `summary` block is the same
totals `llvm-cov report` displays. Routing the LLVM path through JSON
removes the LCOV+--auth-summary patching the renderer otherwise has
to do.

Standard library only — `json` plus `gzip` for `.json.gz` inputs.
"""

import collections
import gzip
import json
from typing import Any, Dict, IO, List, Optional, Sequence, Tuple

from lcov_io import (
    AuthFileSummary,
    FileRecord,
    Function,
    LcovParseError,
    Region,
)


# ---------------------------------------------------------------------------
# Region-kind enum (from llvm/ProfileData/Coverage/CoverageMapping.h)
# ---------------------------------------------------------------------------
#
# The JSON export emits these as the trailing integer in each region
# tuple. We keep the four we care about; the rest are ignored at
# parse time (gap / skipped regions don't contribute to coverage
# totals, and expansion regions come back through `expansions[]`
# anyway, which the per-file summary already accounts for).

_REGION_KIND_CODE = 0
_REGION_KIND_EXPANSION = 1
_REGION_KIND_SKIPPED = 2
_REGION_KIND_GAP = 3
_REGION_KIND_BRANCH = 4
_REGION_KIND_MCDC = 5


# Tuple positions for the `segments` array. Each segment is
# [line, col, count, has_count, is_region_entry, is_gap_region].
_SEG_LINE = 0
_SEG_COL = 1
_SEG_COUNT = 2
_SEG_HAS_COUNT = 3
_SEG_IS_REGION_ENTRY = 4
_SEG_IS_GAP = 5


# Tuple positions for `branches` items per file. Item shape is
# [line_start, col_start, line_end, col_end, count, false_count,
#  file_id, expanded_file_id, kind].
_BR_LINE_START = 0
_BR_COL_START = 1
_BR_COUNT = 4
_BR_FALSE_COUNT = 5


# Tuple positions for `regions` items inside a function. Shape is
# [line_start, col_start, line_end, col_end, execution_count,
#  file_id, expanded_file_id, kind].
_RG_LINE_START = 0
_RG_LINE_END = 2
_RG_COUNT = 4
_RG_FILE_ID = 5
_RG_KIND = 7


def is_json_input(path: str) -> bool:
    """True iff `path` looks like an `llvm-cov export -format=json`
    artifact based on its extension.

    Single source of truth for the JSON-vs-LCOV input policy. Both
    `slang-coverage-html.py` (dispatch to the right parser) and
    `slang-coverage-merge.py` (refuse JSON input) consult this
    helper so the two CLIs can't drift on what counts as JSON.
    """
    lower = path.lower()
    return lower.endswith(".json") or lower.endswith(".json.gz")


def parse_llvm_cov_json(path: str) -> List[FileRecord]:
    """Parse an `llvm-cov export -format=json` document into FileRecords.

    Single-binary input expected. `llvm-cov export` emits one
    `data[]` block per binary passed on its command line; if the
    same source file appears in more than one block (typical when
    a shared header is included by multiple binaries), the later
    block overwrites the earlier one. Slang's pipeline only ever
    invokes `llvm-cov export $LIBSLANG` so duplicates don't occur
    in practice; cross-binary merging belongs upstream of this
    parser if it ever becomes a use case.

    Auto-detects gzip by extension. The resulting records have:

    * `lines` — derived by walking each file's `segments` array;
      one (line → max execution count) entry per executable line.
    * `branches` — populated from each file's `branches` array,
      one dict entry per arm (true / false) so the per-line branch
      gutter can render. Totals come from `auth_override`, not from
      counting these dict entries — `llvm-cov report` collapses
      arms differently.
    * `functions` — one entry per top-level `functions[]` record
      that lists this file in `filenames`. `first_line` taken from
      the function's first non-skipped region.
    * `regions` — code regions only (kind 0); expansion / skipped /
      gap / branch / mcdc regions are filtered out so the count
      matches what `llvm-cov report`'s Regions column shows.
    * `auth_override` — the per-file `summary` block, populating all
      four metric totals so the rendered numbers match `llvm-cov
      report` by construction.
    """
    opener: IO[Any]
    try:
        if path.lower().endswith(".gz"):
            opener = gzip.open(path, "rt", encoding="utf-8-sig")
        else:
            opener = open(path, "r", encoding="utf-8-sig")
    except OSError as e:
        raise LcovParseError(f"cannot open JSON coverage file {path!r}: {e}") from e

    try:
        with opener as fh:
            try:
                doc = json.load(fh)
            except json.JSONDecodeError as e:
                raise LcovParseError(
                    f"{path}: malformed llvm-cov JSON ({e.msg} at line {e.lineno}, col {e.colno})"
                ) from e
    except OSError as e:
        raise LcovParseError(f"error reading {path!r}: {e}") from e

    if not isinstance(doc, dict) or doc.get("type") != "llvm.coverage.json.export":
        raise LcovParseError(
            f"{path}: not an llvm-cov JSON export (missing or wrong "
            f"`type` field)"
        )

    data_blocks = doc.get("data") or []
    if not isinstance(data_blocks, list):
        raise LcovParseError(f"{path}: `data` is not a list")

    records: "collections.OrderedDict[str, FileRecord]" = collections.OrderedDict()

    for block in data_blocks:
        if not isinstance(block, dict):
            continue
        files = block.get("files") or []
        for f in files:
            if not isinstance(f, dict):
                continue
            rec = _file_to_record(f)
            if rec is None:
                continue
            records[rec.path] = rec

        # `functions[]` lives at the data-block level and references
        # files by name. Walk it once per block and dispatch.
        for fn in block.get("functions") or []:
            if not isinstance(fn, dict):
                continue
            _attach_function(fn, records)

    return list(records.values())


def _file_to_record(f: Dict[str, Any]) -> Optional[FileRecord]:
    """Build a FileRecord from one entry under `data[].files[]`."""
    filename = f.get("filename")
    if not isinstance(filename, str) or not filename:
        return None

    rec = FileRecord(path=filename)

    segments = f.get("segments") or []
    rec.lines = _line_hits_from_segments(segments)

    rec.branches = _branches_from_json(f.get("branches") or [])

    summary = f.get("summary") or {}
    rec.auth_override = _summary_to_auth(summary)

    return rec


def _line_hits_from_segments(
    segments: Sequence[Sequence[Any]],
) -> Dict[int, int]:
    """Walk segments to produce {line: max_count} for executable lines.

    Each segment activates a count at (line, col); the next segment
    deactivates or replaces it. The active count covers the half-open
    line interval [seg.line, next_seg.line) — the next segment's start
    line is excluded so a deactivating successor doesn't inherit the
    previous active count under our `max()` rule. For the final
    segment, no successor exists, so we attribute its own start line
    only.

    Gap regions and segments without an associated count are skipped
    — they mark non-executable spans, not zero-hit ones.

    The last segment is typically a closing marker (count 0,
    `has_count=False`) and contributes nothing on its own.
    """
    line_hits: Dict[int, int] = {}
    n = len(segments)
    for i in range(n):
        seg = segments[i]
        # Required fields: line, col, count, has_count, is_region_entry.
        # is_gap_region was added in LLVM (~March 2020) so older
        # exports emit only five fields; treat absent as False.
        if len(seg) < 5:
            continue
        try:
            line = int(seg[_SEG_LINE])
            count = int(seg[_SEG_COUNT])
            has_count = bool(seg[_SEG_HAS_COUNT])
            is_gap = (
                bool(seg[_SEG_IS_GAP]) if len(seg) > _SEG_IS_GAP else False
            )
        except (TypeError, ValueError):
            continue
        if not has_count or is_gap:
            continue
        # Half-open end: next segment's line excluded so it doesn't
        # inherit this segment's count via the max rule below.
        if i + 1 < n and len(segments[i + 1]) >= 2:
            try:
                end_line = int(segments[i + 1][_SEG_LINE])
            except (TypeError, ValueError):
                end_line = line + 1
            if end_line <= line:
                end_line = line + 1
        else:
            end_line = line + 1
        for ln in range(line, end_line):
            prev = line_hits.get(ln)
            if prev is None or count > prev:
                line_hits[ln] = count
    return line_hits


def _branches_from_json(
    branches: Sequence[Sequence[Any]],
) -> Dict[Tuple[int, int, int], Optional[int]]:
    """Populate the FileRecord.branches dict from JSON branch entries.

    Each LLVM branch carries (count, false_count). We emit one dict
    entry per arm so the per-line branch gutter renders both
    outcomes; totals are read from auth_override, not from len() of
    this dict — `llvm-cov report` counts each branch as one unit
    while LCOV BRDA counts each arm as one, and we want the rendered
    total to match the report.

    The dict key is `(line, idx, arm)` where `idx` is the branch's
    index within the JSON `branches[]` array (used as a synthetic
    block id) and `arm` alternates 0 (true path) / 1 (false path).
    A `count` of 0 with non-zero `false_count` (or vice-versa)
    produces a partial-coverage line in the gutter.
    """
    out: Dict[Tuple[int, int, int], Optional[int]] = {}
    for idx, br in enumerate(branches):
        if len(br) < 6:
            continue
        try:
            line = int(br[_BR_LINE_START])
            true_count = int(br[_BR_COUNT])
            false_count = int(br[_BR_FALSE_COUNT])
        except (TypeError, ValueError):
            continue
        out[(line, idx, 0)] = true_count
        out[(line, idx, 1)] = false_count
    return out


def _summary_to_auth(summary: Dict[str, Any]) -> AuthFileSummary:
    """Convert a per-file `summary` dict to an AuthFileSummary."""

    def _pair(key: str) -> Tuple[int, int]:
        block = summary.get(key) or {}
        try:
            count = int(block.get("count", 0))
            covered = int(block.get("covered", 0))
        except (TypeError, ValueError):
            return 0, 0
        missed = max(0, count - covered)
        return count, missed

    line_total, line_missed = _pair("lines")
    func_total, func_missed = _pair("functions")
    branch_total, branch_missed = _pair("branches")
    region_total, region_missed = _pair("regions")

    return AuthFileSummary(
        line_total=line_total,
        line_missed=line_missed,
        func_total=func_total,
        func_missed=func_missed,
        branch_total=branch_total,
        branch_missed=branch_missed,
        region_total=region_total,
        region_missed=region_missed,
    )


def _attach_function(fn: Dict[str, Any], records: Dict[str, FileRecord]) -> None:
    """Attach one top-level functions[] entry to its owning FileRecord.

    Each function has `filenames`, a list of files this function spans
    (multiple entries when the function expands across includes), and
    a `regions` list with per-region execution counts. We attach the
    function to its primary file (file_id 0 when present, else
    filenames[0]) and append its code regions to that file's regions
    list.
    """
    filenames = fn.get("filenames") or []
    if not filenames:
        return
    primary = filenames[0]
    name = fn.get("name") or ""
    try:
        hits = int(fn.get("count", 0))
    except (TypeError, ValueError):
        hits = 0

    rec = records.get(primary)
    if rec is None:
        return

    regions = fn.get("regions") or []

    # First CodeRegion in the primary file drives `first_line`. Mirror
    # the same filter the region-append loop below uses so an
    # EXPANSION region pointing into an included file (file_id != 0)
    # can't set `first_line` to a line that doesn't exist in the
    # primary file.
    first_line = 0
    for r in regions:
        if len(r) <= _RG_KIND:
            continue
        if r[_RG_KIND] != _REGION_KIND_CODE:
            continue
        try:
            file_id = int(r[_RG_FILE_ID])
        except (TypeError, ValueError):
            file_id = 0
        if file_id != 0:
            continue
        try:
            first_line = int(r[_RG_LINE_START])
            break
        except (TypeError, ValueError):
            continue

    if name:
        rec.functions[name] = Function(first_line=first_line, hits=hits)

    # Append code regions to the primary file's regions list. We
    # filter to kind 0 (CodeRegion) so the count matches what
    # `llvm-cov report`'s Regions column shows; expansion regions
    # are accounted for via the per-file summary block already, and
    # gap / skipped / branch / mcdc regions don't enter the totals.
    for r in regions:
        if len(r) <= _RG_KIND:
            continue
        kind = r[_RG_KIND]
        if kind != _REGION_KIND_CODE:
            continue
        try:
            file_id = int(r[_RG_FILE_ID])
        except (TypeError, ValueError):
            file_id = 0
        # file_id != 0 means the region lives in an expansion of an
        # included file, not the function's primary file.
        if file_id != 0:
            continue
        try:
            ls = int(r[_RG_LINE_START])
            le = int(r[_RG_LINE_END])
            count = int(r[_RG_COUNT])
        except (TypeError, ValueError):
            continue
        rec.regions.append(Region(line_start=ls, line_end=le, count=count))
