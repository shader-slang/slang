#!/usr/bin/env python3
"""
slang-coverage-html — render LCOV .info files to static HTML.

Zero-dependency replacement for `genhtml` for Slang-ecosystem
coverage workflows. Consumes standard LCOV `.info`, emits a
self-contained directory of static HTML (no JS framework, no CDN,
no external CSS). Python 3.8+ standard library only.

Usage:
    slang-coverage-html input.info
    slang-coverage-html input.info --output-dir out/ --source-root /repo
    slang-coverage-html --help

See tools/coverage-html/README.md for the full flag list.
"""

import argparse
import collections
import datetime
import fnmatch
import hashlib
import html
import os
import posixpath
import re
import sys
from dataclasses import dataclass, field
from typing import Dict, Iterable, List, Optional, Tuple

MARKER_NAME = "slang-coverage-html.marker"
GENERATOR_NAME = "slang-coverage-html"
GENERATOR_URL = "https://github.com/shader-slang/slang"

# Rate tier thresholds (match genhtml defaults)
TIER_HI = 90.0
TIER_MED = 75.0


# ---------------------------------------------------------------------------
# Data model
# ---------------------------------------------------------------------------


@dataclass
class FileRecord:
    """One coverage record for a single source file.

    `lines` is the aggregated per-line hit count across any TN: blocks
    that referenced this file. `reported_lf/lh` are the sums declared
    by the producer (used only for post-validation).
    """

    path: str
    lines: Dict[int, int] = field(default_factory=dict)
    reported_lf: Optional[int] = None
    reported_lh: Optional[int] = None

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


# ---------------------------------------------------------------------------
# LCOV parser
# ---------------------------------------------------------------------------


class LcovParseError(Exception):
    pass


def parse_lcov(path: str) -> List[FileRecord]:
    """Parse an LCOV .info file into a list of FileRecord.

    Aggregates across multiple TN: blocks and multiple SF: entries
    for the same file path. Unknown record types are logged once each
    (via stderr) and otherwise ignored. Malformed DA records raise
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

            # Strip record prefix.
            tag, sep, value = line.partition(":")
            if not sep:
                # Not "TAG:..." — skip, but log once.
                unknown_seen[tag] += 1
                continue

            if tag == "TN":
                # Test-name marker; phase 1 aggregates, so we ignore.
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
                # Aggregate: take the max across TN: blocks so that a
                # line covered by any test counts as covered.
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
            elif tag in ("BRDA", "BRF", "BRH", "FN", "FNDA", "FNF", "FNH"):
                # Phase 2/3 — parsed but ignored.
                continue
            elif tag in ("VER", "FNL"):
                # Version / function-with-line — ignored.
                continue
            else:
                unknown_seen[tag] += 1

    for tag, count in unknown_seen.items():
        print(
            f"slang-coverage-html: warning: ignored {count} record(s) with "
            f"unrecognized tag {tag!r}",
            file=sys.stderr,
        )

    return list(records.values())


# ---------------------------------------------------------------------------
# Source resolver
# ---------------------------------------------------------------------------


class SourceResolver:
    """Resolve an LCOV SF: path to source text on disk.

    Tries (in order):
      1. direct open of the path as given
      2. source_root / path (if --source-root)
      3. source_root / basename(path)  (basename fallback)

    Caches hits and misses. `load(path)` returns (text, resolved_path)
    or (None, None) on miss.
    """

    def __init__(self, source_root: Optional[str], cwd: str):
        self.source_root = source_root
        self.cwd = cwd
        self._cache: Dict[str, Tuple[Optional[str], Optional[str]]] = {}

    def load(self, path: str) -> Tuple[Optional[str], Optional[str]]:
        if path in self._cache:
            return self._cache[path]
        text, resolved = self._locate(path)
        self._cache[path] = (text, resolved)
        return text, resolved

    def _locate(self, path: str) -> Tuple[Optional[str], Optional[str]]:
        candidates: List[str] = []
        # 1. As-is (absolute or relative to cwd).
        if os.path.isabs(path):
            candidates.append(path)
        else:
            candidates.append(os.path.join(self.cwd, path))
        # 2. source_root / path.
        if self.source_root:
            if os.path.isabs(path):
                # Strip leading slash so join keeps it as a subpath.
                rel = path.lstrip(os.sep).lstrip("/")
                candidates.append(os.path.join(self.source_root, rel))
            else:
                candidates.append(os.path.join(self.source_root, path))
            # 3. source_root / basename fallback (last resort).
            candidates.append(os.path.join(self.source_root, os.path.basename(path)))

        for c in candidates:
            if os.path.isfile(c):
                try:
                    with open(c, "r", encoding="utf-8-sig", errors="replace") as fh:
                        return fh.read(), os.path.abspath(c)
                except OSError:
                    continue
        return None, None


# ---------------------------------------------------------------------------
# HTML generation
# ---------------------------------------------------------------------------


INLINE_CSS = """\
body { color: #000; background-color: #fff; font-family: sans-serif; }
a:link    { color: #284fa8; text-decoration: underline; }
a:visited { color: #00cb40; text-decoration: underline; }
a:active  { color: #ff0040; text-decoration: underline; }

table.chrome { width: 100%; border-collapse: collapse; }
td.title   { text-align: center; padding-bottom: 10px; font-size: 20pt;
             font-style: italic; font-weight: bold; }
td.ruler   { background-color: #6688d4; height: 3px; padding: 0; }

td.headerItem        { text-align: right; padding-right: 6px; font-weight: bold;
                       vertical-align: top; white-space: nowrap; }
td.headerValue       { text-align: left; color: #284fa8; font-weight: bold;
                       white-space: nowrap; }
td.headerCovTableHead { text-align: center; padding: 0 6px; white-space: nowrap; }
td.headerCovTableEntry    { text-align: right; color: #284fa8; font-weight: bold;
                            padding-left: 12px; padding-right: 4px;
                            background-color: #dae7fe; white-space: nowrap; }
td.headerCovTableEntryHi  { text-align: right; color: #000; font-weight: bold;
                            padding-left: 12px; padding-right: 4px;
                            background-color: #a7fc9d; white-space: nowrap; }
td.headerCovTableEntryMed { text-align: right; color: #000; font-weight: bold;
                            padding-left: 12px; padding-right: 4px;
                            background-color: #ffea20; white-space: nowrap; }
td.headerCovTableEntryLo  { text-align: right; color: #000; font-weight: bold;
                            padding-left: 12px; padding-right: 4px;
                            background-color: #ff0000; white-space: nowrap; }

table.indexTable { margin: 0 auto; width: 80%; border-collapse: collapse; }
td.tableHead { text-align: center; color: #fff; background-color: #6688d4;
               font-size: 120%; font-weight: bold; padding: 2px 4px;
               white-space: nowrap; }

td.coverFile       { text-align: left; padding: 2px 20px 2px 10px; color: #284fa8;
                     background-color: #dae7fe; font-family: monospace; }
td.coverDirectory  { text-align: left; padding: 2px 20px 2px 10px; color: #284fa8;
                     background-color: #b8d0ff; font-family: monospace; }
td.coverBar        { padding: 2px 10px; background-color: #dae7fe; }
span.coverBarOutline { display: inline-block; height: 10px; width: 100px;
                       background-color: #000; vertical-align: middle;
                       line-height: 0; }
span.coverBarFill  { display: inline-block; height: 10px; vertical-align: top; }
span.coverBarFillHi  { background-color: #a7fc9d; }
span.coverBarFillMed { background-color: #ffea20; }
span.coverBarFillLo  { background-color: #ff0000; }
span.coverBarRest    { background-color: #fff; display: inline-block;
                       height: 10px; vertical-align: top; }

td.coverPerHi  { text-align: right; padding: 2px 10px; background-color: #a7fc9d;
                 font-weight: bold; }
td.coverPerMed { text-align: right; padding: 2px 10px; background-color: #ffea20;
                 font-weight: bold; }
td.coverPerLo  { text-align: right; padding: 2px 10px; background-color: #ff0000;
                 font-weight: bold; }
td.coverNumDflt { text-align: right; padding: 2px 10px; background-color: #dae7fe;
                  white-space: nowrap; }
td.coverNumHi   { text-align: right; padding: 2px 10px; background-color: #a7fc9d;
                  white-space: nowrap; }
td.coverNumMed  { text-align: right; padding: 2px 10px; background-color: #ffea20;
                  white-space: nowrap; }
td.coverNumLo   { text-align: right; padding: 2px 10px; background-color: #ff0000;
                  white-space: nowrap; }

td.footnote    { padding: 4px 10px; background-color: #dae7fe;
                 font-style: italic; font-size: 85%; }
td.versionInfo { text-align: center; padding-top: 2px; font-style: italic;
                 font-size: 90%; }

pre.sourceHeading { font-family: monospace; font-weight: bold; margin: 0; }
pre.source        { font-family: monospace; margin-top: 2px; }
span.lineNum      { background-color: #efe383; display: inline-block;
                    min-width: 6em; text-align: right; padding-right: 4px; }
span.tlaGNC       { background-color: #CAD7FE; }
span.tlaUNC       { background-color: #FF6230; }

.sourceUnavailable { color: #a33; font-style: italic; padding: 8px; }
"""


def _tier(pct: float) -> str:
    if pct >= TIER_HI:
        return "Hi"
    if pct >= TIER_MED:
        return "Med"
    return "Lo"


def _escape_path_for_filename(relpath: str) -> str:
    """Turn a display path into a safe output filename.

    Sanitizes path separators and leading `..` so the output lands
    flat in --output-dir. Collisions are rare but resolved with a
    short sha1 suffix.
    """
    safe = re.sub(r"[^A-Za-z0-9._-]+", "_", relpath).strip("_") or "file"
    suffix = hashlib.sha1(relpath.encode("utf-8")).hexdigest()[:6]
    return f"{safe}.{suffix}.html"


def _common_dir_prefix(paths: List[str]) -> str:
    """Return the longest common directory prefix across SF: paths.

    Normalized with forward slashes; returns empty string if paths
    don't share a prefix. Only trims to directory boundaries.
    """
    if not paths:
        return ""
    norm = [p.replace("\\", "/") for p in paths]
    common = posixpath.commonprefix(norm)
    # Trim to last '/' to avoid cutting mid-filename.
    cut = common.rfind("/")
    return common[: cut + 1] if cut >= 0 else ""


def _display_path(full: str, prefix: str) -> str:
    """Human-readable label for a file, after stripping common prefix."""
    norm = full.replace("\\", "/")
    if prefix and norm.startswith(prefix):
        return norm[len(prefix) :] or posixpath.basename(norm)
    return norm


def _render_page_header(
    title: str,
    breadcrumb_html: str,
    test_name: str,
    test_date: str,
    overall_pct: float,
    total: int,
    hit: int,
) -> str:
    tier = _tier(overall_pct) if total > 0 else "Hi"
    pct_str = f"{overall_pct:.1f}&nbsp;%" if total > 0 else "-"
    return f"""<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html lang="en">
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
  <title>{html.escape(title)}</title>
  <style>{INLINE_CSS}</style>
</head>
<body>
  <table class="chrome" cellpadding="0" cellspacing="0">
    <tr><td class="title">LCOV - code coverage report</td></tr>
    <tr><td class="ruler"></td></tr>
    <tr>
      <td>
        <table cellpadding="1" border="0" width="100%">
          <tr>
            <td width="10%" class="headerItem">Current view:</td>
            <td width="10%" class="headerValue">{breadcrumb_html}</td>
            <td width="5%"></td>
            <td width="5%"></td>
            <td width="5%" class="headerCovTableHead">Coverage</td>
            <td width="5%" class="headerCovTableHead" title="Covered + Uncovered code">Total</td>
            <td width="5%" class="headerCovTableHead" title="Exercised code only">Hit</td>
          </tr>
          <tr>
            <td class="headerItem">Test:</td>
            <td class="headerValue">{html.escape(test_name)}</td>
            <td></td>
            <td class="headerItem">Lines:</td>
            <td class="headerCovTableEntry{tier}">{pct_str}</td>
            <td class="headerCovTableEntry">{total}</td>
            <td class="headerCovTableEntry">{hit}</td>
          </tr>
          <tr>
            <td class="headerItem">Date:</td>
            <td class="headerValue">{html.escape(test_date)}</td>
            <td></td>
            <td class="headerItem">Functions:</td>
            <td class="headerCovTableEntryHi">-</td>
            <td class="headerCovTableEntry">0</td>
            <td class="headerCovTableEntry">0</td>
          </tr>
        </table>
      </td>
    </tr>
    <tr><td class="ruler"></td></tr>
  </table>
"""


def _render_page_footer() -> str:
    return f"""  <br>
  <table class="chrome" cellpadding="0" cellspacing="0">
    <tr><td class="ruler"></td></tr>
    <tr><td class="versionInfo">Generated by:
      <a href="{GENERATOR_URL}">{GENERATOR_NAME}</a></td></tr>
  </table>
</body>
</html>
"""


def _render_rate_bar(pct: float) -> str:
    tier = _tier(pct)
    fill_w = max(0, min(100, int(round(pct))))
    rest_w = 100 - fill_w
    return (
        f'<span class="coverBarOutline">'
        f'<span class="coverBarFill coverBarFill{tier}" '
        f'style="width:{fill_w}px"></span>'
        f'<span class="coverBarRest" style="width:{rest_w}px"></span>'
        f"</span>"
    )


def render_index(
    records: List[FileRecord],
    output_dir: str,
    title: str,
    test_name: str,
    test_date: str,
    prefix: str,
    file_map: Dict[str, str],
) -> None:
    total = sum(r.total_lines for r in records)
    hit = sum(r.hit_lines for r in records)
    pct = (100.0 * hit / total) if total else 0.0

    header = _render_page_header(
        title=title,
        breadcrumb_html="top level",
        test_name=test_name,
        test_date=test_date,
        overall_pct=pct,
        total=total,
        hit=hit,
    )

    # Table rows, one per file. Sort by display-path for determinism.
    rows_html: List[str] = []
    sorted_records = sorted(records, key=lambda r: _display_path(r.path, prefix))
    for rec in sorted_records:
        display = _display_path(rec.path, prefix)
        href = file_map[rec.path]
        rate = rec.percent
        tier = _tier(rate)
        rate_cell_class = f"coverPer{tier}"
        rate_str = f"{rate:.1f}&nbsp;%" if rec.total_lines else "-"
        rows_html.append(
            f"""            <tr>
              <td class="coverFile"><a href="{html.escape(href)}">{html.escape(display)}</a></td>
              <td class="coverBar" align="center">{_render_rate_bar(rate)}</td>
              <td class="{rate_cell_class}">{rate_str}</td>
              <td class="coverNumDflt">{rec.total_lines}</td>
              <td class="coverNumDflt">{rec.hit_lines}</td>
            </tr>"""
        )

    if not records:
        rows_html.append(
            '            <tr><td colspan="5" class="footnote" '
            'style="text-align:center">No coverage data found in input.</td></tr>'
        )

    prefix_note = ""
    if prefix:
        prefix_note = (
            f'        <tr><td colspan="5" class="footnote">Common path prefix: '
            f'<code>{html.escape(prefix)}</code></td></tr>\n'
        )

    body = f"""  <center>
    <table class="indexTable" cellpadding="1" cellspacing="1" border="0">
      <tr>
        <td width="40%"><br></td>
        <td width="15%"></td><td width="15%"></td>
        <td width="15%"></td><td width="15%"></td>
      </tr>
      <tr>
        <td class="tableHead" rowspan="2">File</td>
        <td class="tableHead" colspan="4">Line Coverage</td>
      </tr>
      <tr>
        <td class="tableHead" colspan="2">Rate</td>
        <td class="tableHead">Total</td>
        <td class="tableHead">Hit</td>
      </tr>
{chr(10).join(rows_html)}
{prefix_note}    </table>
  </center>
"""

    out = header + body + _render_page_footer()
    with open(os.path.join(output_dir, "index.html"), "w", encoding="utf-8") as f:
        f.write(out)


def render_file_page(
    record: FileRecord,
    source_text: Optional[str],
    output_dir: str,
    out_filename: str,
    display_name: str,
    title: str,
    test_name: str,
    test_date: str,
) -> None:
    breadcrumb = (
        f'<a href="index.html" title="Click to go to top-level">top level</a> '
        f"- {html.escape(display_name)}"
    )
    header = _render_page_header(
        title=f"{title} - {display_name}",
        breadcrumb_html=breadcrumb,
        test_name=test_name,
        test_date=test_date,
        overall_pct=record.percent,
        total=record.total_lines,
        hit=record.hit_lines,
    )

    if source_text is not None:
        body = _render_source_view(record, source_text)
    else:
        body = _render_placeholder_view(record)

    out = header + body + _render_page_footer()
    with open(os.path.join(output_dir, out_filename), "w", encoding="utf-8") as f:
        f.write(out)


def _render_source_view(record: FileRecord, source_text: str) -> str:
    # Split source into lines; preserve numbering 1-based.
    src_lines = source_text.splitlines()

    parts: List[str] = ['<table><tr><td>']
    parts.append(
        '<pre class="sourceHeading">'
        '            Line data    Source code'
        '</pre>'
    )
    parts.append('<pre class="source">')
    for idx, raw in enumerate(src_lines, start=1):
        esc = html.escape(raw)
        hits = record.lines.get(idx)
        line_num_cell = f'<span class="lineNum">{idx:>8}</span>'
        if hits is None:
            # Non-executable: no coverage info for this line.
            parts.append(
                f'<span id="L{idx}">{line_num_cell}'
                f'              : {esc}</span>'
            )
        elif hits > 0:
            parts.append(
                f'<span id="L{idx}">{line_num_cell} '
                f'<span class="tlaGNC">{hits:>12} : {esc}</span></span>'
            )
        else:
            parts.append(
                f'<span id="L{idx}">{line_num_cell} '
                f'<span class="tlaUNC">{0:>12} : {esc}</span></span>'
            )
    parts.append("</pre></td></tr></table>")
    return "\n".join(parts) + "\n"


def _render_placeholder_view(record: FileRecord) -> str:
    rows = [
        '  <div class="sourceUnavailable">Source file not found; '
        'coverage data below. Re-run with <code>--source-root</code> to '
        "resolve.</div>"
    ]
    rows.append('  <table class="indexTable" cellpadding="2" cellspacing="1">')
    rows.append(
        '    <tr><td class="tableHead">Line</td><td class="tableHead">Hits</td></tr>'
    )
    for ln in sorted(record.lines):
        hits = record.lines[ln]
        cls = "coverNumDflt" if hits > 0 else "coverNumLo"
        rows.append(
            f'    <tr><td class="coverFile">{ln}</td>'
            f'<td class="{cls}">{hits}</td></tr>'
        )
    rows.append("  </table>")
    return "\n".join(rows) + "\n"


# ---------------------------------------------------------------------------
# Filters + output-dir safety
# ---------------------------------------------------------------------------


def apply_filters(
    records: List[FileRecord],
    includes: List[str],
    excludes: List[str],
) -> List[FileRecord]:
    if not includes and not excludes:
        return records
    out: List[FileRecord] = []
    for r in records:
        p = r.path.replace("\\", "/")
        if includes and not any(fnmatch.fnmatch(p, pat) for pat in includes):
            continue
        if excludes and any(fnmatch.fnmatch(p, pat) for pat in excludes):
            continue
        out.append(r)
    return out


def prepare_output_dir(path: str) -> None:
    """Create `path` if missing. If non-empty, require the marker."""
    if not os.path.exists(path):
        os.makedirs(path)
    elif not os.path.isdir(path):
        raise SystemExit(f"error: {path!r} exists and is not a directory")
    else:
        entries = os.listdir(path)
        if entries and MARKER_NAME not in entries:
            raise SystemExit(
                f"error: output directory {path!r} is non-empty and was not "
                f"written by slang-coverage-html. Refusing to overwrite. "
                f"Pick a different --output-dir or delete the existing one."
            )
    # Drop marker so subsequent runs identify the dir as ours.
    with open(os.path.join(path, MARKER_NAME), "w", encoding="utf-8") as f:
        f.write(f"{GENERATOR_NAME}\n")


# ---------------------------------------------------------------------------
# Validation (post-generation)
# ---------------------------------------------------------------------------


def validate_totals(records: List[FileRecord]) -> None:
    for r in records:
        if r.reported_lf is not None and r.reported_lf != r.total_lines:
            print(
                f"slang-coverage-html: warning: {r.path}: LF reported "
                f"{r.reported_lf} but saw {r.total_lines} DA lines",
                file=sys.stderr,
            )
        if r.reported_lh is not None and r.reported_lh != r.hit_lines:
            print(
                f"slang-coverage-html: warning: {r.path}: LH reported "
                f"{r.reported_lh} but saw {r.hit_lines} hit DA lines",
                file=sys.stderr,
            )


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------


def build_argparser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        prog="slang-coverage-html",
        description="Render an LCOV .info coverage file to static HTML.",
    )
    p.add_argument("input", help="LCOV .info file to render")
    p.add_argument(
        "--output-dir",
        default="coverage-html",
        help="Output directory (default: ./coverage-html/)",
    )
    p.add_argument(
        "--title",
        default="Coverage report",
        help='Page title (default: "Coverage report")',
    )
    p.add_argument(
        "--source-root",
        default=None,
        help="Directory used as a root when resolving SF: paths",
    )
    p.add_argument(
        "--filter-include",
        action="append",
        default=[],
        metavar="GLOB",
        help="Include-only glob (repeatable). Applied to SF: path.",
    )
    p.add_argument(
        "--filter-exclude",
        action="append",
        default=[],
        metavar="GLOB",
        help="Exclude glob (repeatable). Applied to SF: path.",
    )
    p.add_argument(
        "--show-branches",
        action="store_true",
        help="Reserved for phase 2; currently a no-op with a notice.",
    )
    p.add_argument(
        "--show-functions",
        action="store_true",
        help="Reserved for phase 2; currently a no-op with a notice.",
    )
    p.add_argument(
        "--quiet",
        action="store_true",
        help="Suppress progress output",
    )
    return p


def main(argv: Optional[List[str]] = None) -> int:
    args = build_argparser().parse_args(argv)

    if args.show_branches or args.show_functions:
        print(
            "slang-coverage-html: note: --show-branches / --show-functions "
            "are phase-2 features and are currently no-ops.",
            file=sys.stderr,
        )

    try:
        records = parse_lcov(args.input)
    except LcovParseError as e:
        print(f"slang-coverage-html: {e}", file=sys.stderr)
        return 2

    records = apply_filters(records, args.filter_include, args.filter_exclude)
    validate_totals(records)

    prepare_output_dir(args.output_dir)

    prefix = _common_dir_prefix([r.path for r in records])

    # Establish unique output filename per record; handle rare collisions.
    file_map: Dict[str, str] = {}
    used: Dict[str, int] = {}
    for r in records:
        display = _display_path(r.path, prefix)
        name = _escape_path_for_filename(display)
        if name in used:
            used[name] += 1
            root, ext = os.path.splitext(name)
            name = f"{root}-{used[name]}{ext}"
        else:
            used[name] = 0
        file_map[r.path] = name

    test_name = os.path.basename(args.input)
    test_date = datetime.datetime.now(datetime.timezone.utc).strftime(
        "%Y-%m-%d %H:%M:%S UTC"
    )

    resolver = SourceResolver(args.source_root, cwd=os.path.dirname(os.path.abspath(args.input)))

    render_index(
        records=records,
        output_dir=args.output_dir,
        title=args.title,
        test_name=test_name,
        test_date=test_date,
        prefix=prefix,
        file_map=file_map,
    )

    unresolved: List[str] = []
    for r in records:
        source_text, resolved_path = resolver.load(r.path)
        if source_text is None:
            unresolved.append(r.path)
        display = _display_path(r.path, prefix)
        render_file_page(
            record=r,
            source_text=source_text,
            output_dir=args.output_dir,
            out_filename=file_map[r.path],
            display_name=display,
            title=args.title,
            test_name=test_name,
            test_date=test_date,
        )

    if not args.quiet:
        total = sum(r.total_lines for r in records)
        hit = sum(r.hit_lines for r in records)
        pct = (100.0 * hit / total) if total else 0.0
        print(
            f"slang-coverage-html: wrote {len(records)} file page(s) to "
            f"{args.output_dir}/ — overall {pct:.1f}% ({hit}/{total})",
            file=sys.stderr,
        )
        if unresolved:
            print(
                f"slang-coverage-html: note: {len(unresolved)} source file(s) "
                f"were not found on disk; rendered with placeholder content. "
                f"Pass --source-root to help resolve them.",
                file=sys.stderr,
            )

    return 0


if __name__ == "__main__":
    sys.exit(main())
