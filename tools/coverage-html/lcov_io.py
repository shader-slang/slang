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
import os
import sys
from dataclasses import dataclass, field
from typing import Any, Dict, IO, Iterable, List, Optional, Tuple


class SourceResolver:
    """Resolve an LCOV SF: path to source text on disk.

    Tries (in order):
      1. direct open of the path as given (absolute) or relative to
         the LCOV file's directory
      2. relative to the user's invocation cwd — covers the common
         case of running the tool from a repo root with a merged
         LCOV that lives in /tmp
      3. source_root / path (if --source-root)
      4. source_root / basename(path)  (basename fallback)

    Caches hits and misses. `load(path)` returns (text, resolved_path)
    or (None, None) on miss.
    """

    def __init__(
        self,
        source_root: Optional[str],
        cwd: str,
        invocation_cwd: Optional[str] = None,
    ):
        self.source_root = source_root
        self.cwd = cwd
        self.invocation_cwd = invocation_cwd or os.getcwd()
        self._cache: Dict[str, Tuple[Optional[str], Optional[str]]] = {}

    def load(self, path: str) -> Tuple[Optional[str], Optional[str]]:
        if path in self._cache:
            return self._cache[path]
        text, resolved = self._locate(path)
        self._cache[path] = (text, resolved)
        return text, resolved

    def _locate(self, path: str) -> Tuple[Optional[str], Optional[str]]:
        candidates: List[str] = []
        if os.path.isabs(path):
            candidates.append(path)
        else:
            candidates.append(os.path.join(self.cwd, path))
            if self.invocation_cwd != self.cwd:
                candidates.append(os.path.join(self.invocation_cwd, path))
        if self.source_root:
            if os.path.isabs(path):
                rel = path.lstrip(os.sep).lstrip("/")
                candidates.append(os.path.join(self.source_root, rel))
            else:
                candidates.append(os.path.join(self.source_root, path))
            candidates.append(
                os.path.join(self.source_root, os.path.basename(path))
            )
        for c in candidates:
            if os.path.isfile(c):
                try:
                    with open(c, "r", encoding="utf-8-sig", errors="replace") as fh:
                        return fh.read(), os.path.abspath(c)
                except OSError:
                    continue
        return None, None


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
class AuthFileSummary:
    """Per-file totals from the `llvm-cov report` text dump.

    These are the numbers CI's coverage dashboard quotes — they differ
    from a verbose `llvm-cov export -format=lcov` output because
    `llvm-cov report` collapses templated/inlined entries that the
    LCOV form keeps separate. When supplied via the `--auth-summary`
    flag, they override LCOV-derived totals on FileRecord.
    """

    line_total: int = 0
    line_missed: int = 0
    func_total: int = 0
    func_missed: int = 0
    branch_total: int = 0
    branch_missed: int = 0

    @property
    def line_hit(self) -> int:
        return self.line_total - self.line_missed

    @property
    def func_hit(self) -> int:
        return self.func_total - self.func_missed

    @property
    def branch_hit(self) -> int:
        return self.branch_total - self.branch_missed


@dataclass
class AuthSummary:
    """Set of per-file authoritative summaries plus the TOTAL row."""

    files: Dict[str, AuthFileSummary] = field(default_factory=dict)
    total: AuthFileSummary = field(default_factory=AuthFileSummary)

    def get(self, path: str) -> Optional[AuthFileSummary]:
        return self.files.get(path)


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
    # When set (via --auth-summary), the totals/percent properties
    # below return values from the authoritative `llvm-cov report`
    # numbers instead of the LCOV-derived ones. The lines / branches /
    # functions dicts themselves are NOT overridden — per-line and
    # per-function rendering keeps using the LCOV detail.
    auth_override: Optional[AuthFileSummary] = None

    # --- line coverage ---
    @property
    def total_lines(self) -> int:
        if self.auth_override is not None:
            return self.auth_override.line_total
        return len(self.lines)

    @property
    def hit_lines(self) -> int:
        if self.auth_override is not None:
            return self.auth_override.line_hit
        return sum(1 for h in self.lines.values() if h > 0)

    @property
    def percent(self) -> float:
        t = self.total_lines
        return 100.0 * self.hit_lines / t if t else 0.0

    # --- branch coverage ---
    @property
    def total_branches(self) -> int:
        if self.auth_override is not None:
            return self.auth_override.branch_total
        return len(self.branches)

    @property
    def hit_branches(self) -> int:
        if self.auth_override is not None:
            return self.auth_override.branch_hit
        # A branch is "hit" when it was evaluated at least once and
        # the condition took the edge at least once. `None` → not
        # evaluated; `0` → evaluated but never taken.
        return sum(1 for t in self.branches.values() if t is not None and t > 0)

    @property
    def percent_branches(self) -> float:
        t = self.total_branches
        return 100.0 * self.hit_branches / t if t else 0.0

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
        if self.auth_override is not None:
            return self.auth_override.func_total
        line_set, _hit_line_set, orphan_total, _ = self._function_buckets()
        return len(line_set) + orphan_total

    @property
    def hit_functions(self) -> int:
        if self.auth_override is not None:
            return self.auth_override.func_hit
        _line_set, hit_line_set, _, orphan_hit = self._function_buckets()
        return len(hit_line_set) + orphan_hit

    @property
    def percent_functions(self) -> float:
        total = self.total_functions
        if total == 0:
            return 0.0
        return 100.0 * self.hit_functions / total


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

        # LF/LH last. Report the *derived* totals — auth_override
        # is for display, not for round-tripping through LCOV.
        if r.lines:
            out.write(f"LF:{len(r.lines)}\n")
            out.write(
                "LH:"
                f"{sum(1 for h in r.lines.values() if h > 0)}\n"
            )

        out.write("end_of_record\n")


# ---------------------------------------------------------------------------
# llvm-cov report text parser / merger / writer
# ---------------------------------------------------------------------------
#
# Companion to `llvm-cov export -format=lcov`: a CI artifact that
# captures the text published by `llvm-cov report`. Used by the
# `--auth-summary` flag on the renderer and merger to override
# LCOV-derived totals with the authoritative numbers CI's coverage
# dashboard quotes. See `tools/coverage-html/README.md` "Branch
# coverage methodology".

# Column order (after Filename) in `llvm-cov report` text:
#   Regions, MissedRegions, Cover%, Functions, MissedFunctions,
#   Executed%, Lines, MissedLines, Cover%, Branches, MissedBranches,
#   Cover%
# Cover/Executed cells are `-` when the corresponding total is zero.
# We only consume the integer columns (4, 5, 7, 8, 10, 11) and
# recompute percentages from total/missed.


def parse_llvm_cov_report(path: str) -> AuthSummary:
    """Parse an `llvm-cov report` text dump into an AuthSummary."""
    summary = AuthSummary()
    try:
        with open(path, "r", encoding="utf-8-sig", errors="replace") as fh:
            text = fh.read()
    except OSError as e:
        raise LcovParseError(f"cannot open report file {path!r}: {e}")

    for raw in text.splitlines():
        line = raw.rstrip()
        if not line:
            continue
        stripped = line.lstrip()
        if stripped.startswith("---"):
            continue
        if stripped.startswith("Filename"):
            continue
        # Split off the trailing 12 fields. With `None` separator,
        # rsplit collapses runs of whitespace, which is what we want.
        parts = line.rsplit(None, 12)
        if len(parts) != 13:
            continue
        try:
            funcs = int(parts[4])
            miss_funcs = int(parts[5])
            lines_ = int(parts[7])
            miss_lines = int(parts[8])
            brs = int(parts[10])
            miss_brs = int(parts[11])
        except ValueError:
            continue
        fs = AuthFileSummary(
            line_total=lines_,
            line_missed=miss_lines,
            func_total=funcs,
            func_missed=miss_funcs,
            branch_total=brs,
            branch_missed=miss_brs,
        )
        name = parts[0].strip()
        if name == "TOTAL":
            summary.total = fs
        else:
            summary.files[name] = fs
    return summary


def _merge_file_summaries(items: List[AuthFileSummary]) -> AuthFileSummary:
    """Combine a list of summaries by max(total) and min(missed).

    For a single file across N OSes: merged.total is the largest
    line/func/branch count any OS reported (union of source locations
    after #ifdef expansion); merged.missed is the smallest count any
    OS reported (best coverage). The pair is an upper bound — true
    merged numbers from a max-aggregated LCOV may be slightly tighter,
    but they're not derivable from per-OS report text alone.
    """
    if not items:
        return AuthFileSummary()
    line_items = [i for i in items if i.line_total > 0]
    func_items = [i for i in items if i.func_total > 0]
    br_items = [i for i in items if i.branch_total > 0]
    return AuthFileSummary(
        line_total=max((i.line_total for i in items), default=0),
        line_missed=min((i.line_missed for i in line_items), default=0),
        func_total=max((i.func_total for i in items), default=0),
        func_missed=min((i.func_missed for i in func_items), default=0),
        branch_total=max((i.branch_total for i in items), default=0),
        branch_missed=min((i.branch_missed for i in br_items), default=0),
    )


def merge_auth_summaries(summaries: List[AuthSummary]) -> AuthSummary:
    """Merge multiple AuthSummaries (one per OS, typically) into one."""
    out = AuthSummary()
    paths: set = set()
    for s in summaries:
        paths.update(s.files.keys())
    for p in sorted(paths):
        out.files[p] = _merge_file_summaries(
            [s.files[p] for s in summaries if p in s.files]
        )
    totals = [
        s.total
        for s in summaries
        if s.total.line_total or s.total.func_total or s.total.branch_total
    ]
    if totals:
        out.total = _merge_file_summaries(totals)
    return out


def write_llvm_cov_report(summary: AuthSummary, out: IO[str]) -> None:
    """Serialize an AuthSummary back to `llvm-cov report`-shaped text.

    Used by the merger to emit a combined per-file summary derived
    from per-OS report text. The Regions column is not tracked in our
    data model, so we emit Lines/MissedLines as filler in those slots
    — the `parse_llvm_cov_report` consumer ignores the Regions
    columns. Percentages are computed from total/missed and emit `-`
    when the corresponding total is zero.
    """

    def _pct(hit: int, total: int) -> str:
        return "-" if total <= 0 else f"{100.0 * hit / total:.2f}%"

    def _row(name: str, fs: AuthFileSummary) -> str:
        # Filler Regions/MissedRegions columns mirror Lines/MissedLines.
        regions = fs.line_total
        miss_regions = fs.line_missed
        return (
            f"{name:<100}"
            f" {regions:>10} {miss_regions:>17} {_pct(regions - miss_regions, regions):>9}"
            f" {fs.func_total:>11} {fs.func_missed:>17} {_pct(fs.func_hit, fs.func_total):>9}"
            f" {fs.line_total:>11} {fs.line_missed:>17} {_pct(fs.line_hit, fs.line_total):>9}"
            f" {fs.branch_total:>11} {fs.branch_missed:>17} {_pct(fs.branch_hit, fs.branch_total):>9}\n"
        )

    has_total = (
        summary.total.line_total
        or summary.total.func_total
        or summary.total.branch_total
    )

    header = (
        f"{'Filename':<100}"
        f" {'Regions':>10} {'Missed Regions':>17} {'Cover':>9}"
        f" {'Functions':>11} {'Missed Functions':>17} {'Executed':>9}"
        f" {'Lines':>11} {'Missed Lines':>17} {'Cover':>9}"
        f" {'Branches':>11} {'Missed Branches':>17} {'Cover':>9}\n"
    )
    out.write(header)
    out.write("-" * 269 + "\n")
    for name, fs in sorted(summary.files.items()):
        out.write(_row(name, fs))
    if has_total:
        out.write("-" * 269 + "\n")
        out.write(_row("TOTAL", summary.total))
