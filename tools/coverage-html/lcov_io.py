"""
LCOV `.info` parser, writer, and in-memory data model.

Shared by `slang-coverage-html` (the renderer) and
`slang-coverage-merge` (the multi-LCOV merger). Standard library
only; nothing in here knows about HTML or rendering.

The `parse_lcov` function aggregates within a single file by `max`
across `TN:` blocks — a line/branch/function counted by any test
in that file counts as hit. Cross-file aggregation (e.g. merging
LCOVs produced by different OSes) is the merge tool's job; this
module just exposes a clean data model on which the merger can
operate.
"""

import collections
import sys
from dataclasses import dataclass, field
from typing import Any, Dict, IO, Iterable, List, Optional, Tuple


# ---------------------------------------------------------------------------
# Data model
# ---------------------------------------------------------------------------


@dataclass
class Function:
    """One function-level coverage entry.

    `first_line` is the declaring line; `hits` is the call count
    aggregated across any TN: blocks that reported it.
    """

    first_line: int
    hits: int = 0


@dataclass
class FileRecord:
    """One coverage record for a single source file.

    - `lines`    — aggregated per-line hit count across TN: blocks
                   (DA: records).
    - `branches` — {(line, block, branch_id) -> taken_count or None}
                   aggregated across TN: blocks (BRDA:). None means the
                   branch was not executed; integers mean it was
                   executed at least once and taken `n` times.
    - `functions` — {mangled_name -> Function} (FN: + FNDA:).

    Reported LF/LH/BRF/BRH/FNF/FNH are stored for post-validation.
    """

    path: str
    lines: Dict[int, int] = field(default_factory=dict)
    branches: Dict[Tuple[int, int, int], Optional[int]] = field(default_factory=dict)
    functions: Dict[str, Function] = field(default_factory=dict)
    reported_lf: Optional[int] = None
    reported_lh: Optional[int] = None
    reported_brf: Optional[int] = None
    reported_brh: Optional[int] = None
    reported_fnf: Optional[int] = None
    reported_fnh: Optional[int] = None

    # --- line coverage ---
    @property
    def total_lines(self) -> int:
        return len(self.lines)

    @property
    def hit_lines(self) -> int:
        return sum(1 for h in self.lines.values() if h > 0)

    @property
    def percent(self) -> float:
        if not self.lines:
            return 0.0
        return 100.0 * self.hit_lines / self.total_lines

    # --- branch coverage ---
    @property
    def total_branches(self) -> int:
        return len(self.branches)

    @property
    def hit_branches(self) -> int:
        # A branch is "hit" when it was evaluated at least once and
        # the condition took the edge at least once. `None` → not
        # evaluated; `0` → evaluated but never taken.
        return sum(1 for t in self.branches.values() if t is not None and t > 0)

    @property
    def percent_branches(self) -> float:
        if not self.branches:
            return 0.0
        return 100.0 * self.hit_branches / self.total_branches

    # --- function coverage ---
    #
    # NOTE on counting: LCOV emits one FN/FNDA pair per *mangled name*,
    # so template instantiations, implicit copy/move constructors, and
    # other compiler-generated duplicates of a single source function
    # appear as multiple FN records — typically all sharing the same
    # `first_line`. The percentages a user wants reported are
    # "fraction of *source-level* functions covered", which is what
    # `llvm-cov report` computes on the same `.profdata`. We match it
    # by deduping FN records on `(first_line)` per file: any
    # instantiation hit counts the underlying function as hit.
    #
    # Functions with first_line == 0 (orphan FNDA-without-FN — possible
    # but rare from llvm-cov export) can't be deduped by line, so they
    # each count as their own entry by mangled name.

    def _effective_fn_hit(self) -> Dict[str, bool]:
        """For each function, return True if it should be treated as
        covered for reporting purposes — either FNDA reports calls
        OR any DA line inside the function's source range was hit.

        This bridges a real-world LCOV quirk: when the compiler
        inlines a callee at every call site (typical for
        constructors / destructors / RAII helpers in headers), the
        FNDA hit count stays 0 even though the body code executed,
        because llvm's coverage instrumentation tracks function
        entries via dedicated counters that no inlined call ever
        triggers. Pure FNDA accounting then reports 0/N for files
        that LINE coverage shows as 100 % — confusing.
        """
        result: Dict[str, bool] = {}
        items = sorted(
            ((n, fn) for n, fn in self.functions.items() if fn.first_line > 0),
            key=lambda kv: kv[1].first_line,
        )
        sorted_lines = sorted(self.lines.items())
        for i, (name, fn) in enumerate(items):
            if fn.hits > 0:
                result[name] = True
                continue
            start = fn.first_line
            end = items[i + 1][1].first_line if i + 1 < len(items) else 10**9
            any_hit = False
            for ln, hits in sorted_lines:
                if ln < start:
                    continue
                if ln >= end:
                    break
                if hits > 0:
                    any_hit = True
                    break
            result[name] = any_hit
        # Orphan FNDAs (first_line==0): can only use FNDA counts.
        for name, fn in self.functions.items():
            if name not in result:
                result[name] = fn.hits > 0
        return result

    def _function_buckets(self) -> Tuple[set, set]:
        """Return (all_first_lines, hit_first_lines, orphan_total,
        orphan_hit). Hit status uses `_effective_fn_hit`."""
        line_set: set = set()
        hit_line_set: set = set()
        orphan_total = 0
        orphan_hit = 0
        eff = self._effective_fn_hit()
        for name, fn in self.functions.items():
            if fn.first_line > 0:
                line_set.add(fn.first_line)
                if eff.get(name, False):
                    hit_line_set.add(fn.first_line)
            else:
                orphan_total += 1
                if eff.get(name, False):
                    orphan_hit += 1
        # Encode orphan count by extending the sets with synthetic keys.
        # Easier: return the totals separately.
        return line_set, hit_line_set, orphan_total, orphan_hit  # type: ignore[return-value]

    @property
    def total_functions(self) -> int:
        line_set, _hit_line_set, orphan_total, _ = self._function_buckets()
        return len(line_set) + orphan_total

    @property
    def hit_functions(self) -> int:
        _line_set, hit_line_set, _, orphan_hit = self._function_buckets()
        return len(hit_line_set) + orphan_hit

    @property
    def percent_functions(self) -> float:
        total = self.total_functions
        if total == 0:
            return 0.0
        return 100.0 * self.hit_functions / total


# ---------------------------------------------------------------------------
# slangc-only filter
# ---------------------------------------------------------------------------
#
# Mirrors `tools/coverage/slangc-ignore-patterns.sh`, the shared
# definition CI uses for the "slangc compiler-only" coverage view.
# Patterns match repo-relative paths (forward slashes); apply after
# `slang-coverage-merge`'s path normalization, or after manually
# converting absolute / backslash paths.
#
# Each entry is a Python regex that, when found anywhere in the path,
# means the file should be excluded from the slangc-only report.

import re as _re

SLANGC_EXCLUDE_PATTERNS: Tuple[str, ...] = (
    r"build/prelude/",
    r"build/source/slang/(capability|fiddle|slang-lookup-tables)/",
    r"build/source/slang-core-module/",
    r"external/",
    r"include/",
    r"source/slang-core-module/",
    r"source/slang-glslang/",
    r"source/slang-record-replay/",
    r"tools/",
    r"source/slang/slang-(language-server|doc-markdown-writer|doc-ast|ast-dump|repro|workspace-version)[.\-]",
    r"source/slang/slang-ast-(expr|modifier|stmt)\.h$",
)

_SLANGC_RE = _re.compile("|".join(SLANGC_EXCLUDE_PATTERNS))


def is_slangc_filtered_out(path: str) -> bool:
    """True if `path` matches any slangc exclude pattern.

    `path` should be repo-relative with forward slashes — same shape
    as `slang-coverage-merge` produces. Backslashes are normalized
    here for safety.
    """
    return bool(_SLANGC_RE.search(path.replace("\\", "/")))


def apply_slangc_filter(records: List[FileRecord]) -> List[FileRecord]:
    """Return records whose path is NOT excluded by the slangc patterns."""
    return [r for r in records if not is_slangc_filtered_out(r.path)]


def function_branch_coverage(record: FileRecord) -> Dict[str, Tuple[int, int]]:
    """For each function in `record`, compute (total_branches,
    hit_branches) from the BRDA records that fall in the function's
    source range, using the same first_line-based partition as
    `function_line_coverage`.

    Returns `(0, 0)` for functions with `first_line == 0` (orphan
    FNDA without FN) or for files without BRDA records.
    """
    by_name: Dict[str, Tuple[int, int]] = {}
    items = [
        (name, fn) for name, fn in record.functions.items() if fn.first_line > 0
    ]
    items.sort(key=lambda kv: (kv[1].first_line, kv[0]))
    if not items:
        return {name: (0, 0) for name in record.functions}

    # Sort BRDA keys once so we can range-scan each function.
    sorted_branches = sorted(record.branches.items())  # [((line, blk, br), taken), ...]

    for i, (name, fn) in enumerate(items):
        start = fn.first_line
        end = items[i + 1][1].first_line if i + 1 < len(items) else 10**9
        total = 0
        hit = 0
        for (br_line, _block, _branch_id), taken in sorted_branches:
            if br_line < start:
                continue
            if br_line >= end:
                break
            total += 1
            if taken is not None and taken > 0:
                hit += 1
        by_name[name] = (total, hit)
    for name in record.functions:
        by_name.setdefault(name, (0, 0))
    return by_name


def function_line_coverage(record: FileRecord) -> Dict[str, Tuple[int, int]]:
    """For each function in `record`, compute (total_lines, hit_lines)
    from the DA records that fall in the function's source range.

    LCOV doesn't carry a function's end-line. We approximate the
    range as `[first_line, next_function_first_line)` where "next"
    means the next function in source order; the last function picks
    up all remaining DA lines.

    Functions with `first_line == 0` (FNDA reported without an FN
    declaration) report `(0, 0)` — there is no source range to attach
    them to.
    """
    by_name: Dict[str, Tuple[int, int]] = {}

    # Sort functions by first_line, breaking ties on name for
    # determinism. Skip functions without a declared first line.
    items = [
        (name, fn) for name, fn in record.functions.items() if fn.first_line > 0
    ]
    items.sort(key=lambda kv: (kv[1].first_line, kv[0]))

    if not items:
        return {name: (0, 0) for name in record.functions}

    sorted_lines = sorted(record.lines.items())  # [(line, hits), ...]

    for i, (name, fn) in enumerate(items):
        start = fn.first_line
        end = items[i + 1][1].first_line if i + 1 < len(items) else 10**9
        total = 0
        hit = 0
        for ln, hits in sorted_lines:
            if ln < start:
                continue
            if ln >= end:
                break
            total += 1
            if hits > 0:
                hit += 1
        by_name[name] = (total, hit)

    # Fill in (0, 0) for any functions we skipped (first_line == 0).
    for name in record.functions:
        by_name.setdefault(name, (0, 0))
    return by_name


# ---------------------------------------------------------------------------
# LCOV parser
# ---------------------------------------------------------------------------


class LcovParseError(Exception):
    pass


def parse_lcov(
    path: str,
    *,
    warn_prefix: str = "slang-coverage-html",
) -> List[FileRecord]:
    """Parse an LCOV .info file into a list of FileRecord.

    Aggregates across multiple TN: blocks and multiple SF: entries
    for the same file path. Unknown record types are logged once each
    (via stderr) and otherwise ignored. Malformed records raise
    LcovParseError with the offending line number.
    """
    records: "collections.OrderedDict[str, FileRecord]" = collections.OrderedDict()
    current: Optional[FileRecord] = None
    unknown_seen: Dict[str, int] = collections.defaultdict(int)

    try:
        f = open(path, "r", encoding="utf-8-sig", errors="replace")
    except OSError as e:
        raise LcovParseError(f"cannot open LCOV file {path!r}: {e}")

    with f:
        for lineno, raw in enumerate(f, start=1):
            line = raw.strip()
            if not line:
                continue
            if line == "end_of_record":
                current = None
                continue

            tag, sep, value = line.partition(":")
            if not sep:
                unknown_seen[tag] += 1
                continue

            if tag == "TN":
                continue
            elif tag == "SF":
                key = value
                if key in records:
                    current = records[key]
                else:
                    current = FileRecord(path=key)
                    records[key] = current
            elif tag == "DA":
                if current is None:
                    raise LcovParseError(
                        f"{path}:{lineno}: DA record outside SF block"
                    )
                parts = value.split(",")
                if len(parts) < 2:
                    raise LcovParseError(
                        f"{path}:{lineno}: malformed DA record {value!r}"
                    )
                try:
                    ln = int(parts[0])
                    hits = int(parts[1])
                except ValueError:
                    raise LcovParseError(
                        f"{path}:{lineno}: non-integer DA fields {value!r}"
                    )
                prev = current.lines.get(ln, 0)
                current.lines[ln] = max(prev, hits)
            elif tag == "LF":
                if current is not None:
                    try:
                        current.reported_lf = int(value)
                    except ValueError:
                        pass
            elif tag == "LH":
                if current is not None:
                    try:
                        current.reported_lh = int(value)
                    except ValueError:
                        pass
            elif tag == "BRDA":
                if current is None:
                    raise LcovParseError(
                        f"{path}:{lineno}: BRDA record outside SF block"
                    )
                parts = value.split(",")
                if len(parts) < 4:
                    raise LcovParseError(
                        f"{path}:{lineno}: malformed BRDA record {value!r}"
                    )
                try:
                    br_line = int(parts[0])
                    block = int(parts[1])
                    branch_id = int(parts[2])
                except ValueError:
                    raise LcovParseError(
                        f"{path}:{lineno}: non-integer BRDA fields {value!r}"
                    )
                taken_raw = parts[3]
                taken: Optional[int]
                if taken_raw == "-":
                    taken = None
                else:
                    try:
                        taken = int(taken_raw)
                    except ValueError:
                        raise LcovParseError(
                            f"{path}:{lineno}: non-integer BRDA taken "
                            f"field {taken_raw!r}"
                        )
                key = (br_line, block, branch_id)
                prev = current.branches.get(key, None) if key in current.branches else None
                if key not in current.branches:
                    current.branches[key] = taken
                else:
                    if taken is None:
                        pass
                    elif prev is None:
                        current.branches[key] = taken
                    else:
                        current.branches[key] = max(prev, taken)
            elif tag == "BRF":
                if current is not None:
                    try:
                        current.reported_brf = int(value)
                    except ValueError:
                        pass
            elif tag == "BRH":
                if current is not None:
                    try:
                        current.reported_brh = int(value)
                    except ValueError:
                        pass
            elif tag == "FN":
                if current is None:
                    raise LcovParseError(
                        f"{path}:{lineno}: FN record outside SF block"
                    )
                first_line_str, comma, name = value.partition(",")
                if not comma:
                    raise LcovParseError(
                        f"{path}:{lineno}: malformed FN record {value!r}"
                    )
                try:
                    first_line = int(first_line_str)
                except ValueError:
                    raise LcovParseError(
                        f"{path}:{lineno}: non-integer FN line {value!r}"
                    )
                if name not in current.functions:
                    current.functions[name] = Function(first_line=first_line, hits=0)
            elif tag == "FNDA":
                if current is None:
                    raise LcovParseError(
                        f"{path}:{lineno}: FNDA record outside SF block"
                    )
                hits_str, comma, name = value.partition(",")
                if not comma:
                    raise LcovParseError(
                        f"{path}:{lineno}: malformed FNDA record {value!r}"
                    )
                try:
                    hits = int(hits_str)
                except ValueError:
                    raise LcovParseError(
                        f"{path}:{lineno}: non-integer FNDA hits {value!r}"
                    )
                fn = current.functions.get(name)
                if fn is None:
                    fn = Function(first_line=0, hits=0)
                    current.functions[name] = fn
                fn.hits = max(fn.hits, hits)
            elif tag == "FNF":
                if current is not None:
                    try:
                        current.reported_fnf = int(value)
                    except ValueError:
                        pass
            elif tag == "FNH":
                if current is not None:
                    try:
                        current.reported_fnh = int(value)
                    except ValueError:
                        pass
            elif tag in ("VER", "FNL"):
                continue
            else:
                unknown_seen[tag] += 1

    for tag, count in unknown_seen.items():
        print(
            f"{warn_prefix}: warning: ignored {count} record(s) with "
            f"unrecognized tag {tag!r}",
            file=sys.stderr,
        )

    return list(records.values())


# ---------------------------------------------------------------------------
# LCOV writer
# ---------------------------------------------------------------------------


def write_lcov(
    records: Iterable[FileRecord],
    out: IO[str],
    *,
    test_name: Optional[str] = None,
) -> None:
    """Serialize FileRecords to standard LCOV `.info` format.

    - `test_name` becomes the `TN:` line at the top of each block;
      omit (default) to emit a single empty TN: per LCOV convention.
    - Computed totals (LF/LH/BRF/BRH/FNF/FNH) are recomputed from
      the in-memory model, NOT taken from `reported_*` fields.
    - Record ordering inside each block matches the LCOV de-facto
      ordering: TN, SF, FN/FNDA pairs, FNF/FNH, DA, LF/LH, BRDA,
      BRF/BRH, end_of_record.
    """
    tn_value = test_name if test_name is not None else ""
    for r in records:
        out.write(f"TN:{tn_value}\n")
        out.write(f"SF:{r.path}\n")

        # FN: lines first (declaring line + name).
        for name, fn in r.functions.items():
            out.write(f"FN:{fn.first_line},{name}\n")
        # FNDA: lines second (hits + name).
        for name, fn in r.functions.items():
            out.write(f"FNDA:{fn.hits},{name}\n")
        if r.functions:
            out.write(f"FNF:{r.total_functions}\n")
            out.write(f"FNH:{r.hit_functions}\n")

        # DA: per source line.
        for ln in sorted(r.lines):
            out.write(f"DA:{ln},{r.lines[ln]}\n")

        # BRDA: per (line, block, branch).
        for (ln, block, branch_id), taken in sorted(r.branches.items()):
            taken_str = "-" if taken is None else str(taken)
            out.write(f"BRDA:{ln},{block},{branch_id},{taken_str}\n")
        if r.branches:
            out.write(f"BRF:{r.total_branches}\n")
            out.write(f"BRH:{r.hit_branches}\n")

        # LF/LH last.
        if r.lines:
            out.write(f"LF:{r.total_lines}\n")
            out.write(f"LH:{r.hit_lines}\n")

        out.write("end_of_record\n")
