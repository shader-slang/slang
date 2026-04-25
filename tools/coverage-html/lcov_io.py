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
    @property
    def total_functions(self) -> int:
        return len(self.functions)

    @property
    def hit_functions(self) -> int:
        return sum(1 for fn in self.functions.values() if fn.hits > 0)

    @property
    def percent_functions(self) -> float:
        if not self.functions:
            return 0.0
        return 100.0 * self.hit_functions / self.total_functions


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
