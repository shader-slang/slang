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
import datetime
import fnmatch
import hashlib
import html
import os
import posixpath
import re
import sys
from dataclasses import dataclass
from typing import Dict, List, Optional, Tuple

# Shared LCOV parser / writer / data model. Sibling module so both the
# renderer and the merge tool import the same code.
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from lcov_io import (  # noqa: E402
    FileRecord,
    Function,
    LcovParseError,
    apply_slangc_filter,
    parse_lcov,
)

MARKER_NAME = "slang-coverage-html.marker"
GENERATOR_NAME = "slang-coverage-html"
GENERATOR_URL = "https://github.com/shader-slang/slang"

# Rate tier thresholds (match genhtml defaults)
TIER_HI = 90.0
TIER_MED = 75.0


# ---------------------------------------------------------------------------
# Source resolver
# ---------------------------------------------------------------------------


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
        # Where the user invoked the tool from. Often the repo root,
        # which is the right base for repo-relative SF: paths emitted
        # by `slang-coverage-merge`.
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
        # 1. As-is (absolute or relative to LCOV-dir).
        if os.path.isabs(path):
            candidates.append(path)
        else:
            candidates.append(os.path.join(self.cwd, path))
            # 2. Relative to the user's invocation cwd. Distinct from
            #    self.cwd (LCOV-dir): merged LCOVs typically sit in
            #    /tmp while the user runs the tool from the repo root.
            if self.invocation_cwd != self.cwd:
                candidates.append(os.path.join(self.invocation_cwd, path))
        # 3. source_root / path.
        if self.source_root:
            if os.path.isabs(path):
                rel = path.lstrip(os.sep).lstrip("/")
                candidates.append(os.path.join(self.source_root, rel))
            else:
                candidates.append(os.path.join(self.source_root, path))
            # 4. source_root / basename fallback (last resort).
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
/* Bar fill is a continuous gradient from red (0%) → green (100%);
   the per-row inline `style="background-color: hsl(...)"` selects
   the spot on the gradient for that file. */
span.coverBarFill  { display: inline-block; height: 10px; vertical-align: top; }
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

td.coverFn        { text-align: left; padding: 2px 20px 2px 10px; color: #284fa8;
                    background-color: #dae7fe; font-family: monospace;
                    word-break: break-all; }
td.coverFnHi      { text-align: right; padding: 2px 10px;
                    background-color: #a7fc9d; font-weight: bold;
                    white-space: nowrap; }
td.coverFnLo      { text-align: right; padding: 2px 10px;
                    background-color: #ff6230; font-weight: bold;
                    white-space: nowrap; }

span.branchAll   { background-color: #a7fc9d; }
span.branchPart  { background-color: #ffea20; }
span.branchNone  { background-color: #FF6230; }

/* Per-file expand chevron in the index. The placeholder variant
   reserves the same column width on rows that have no functions, so
   the file-name column stays vertically aligned across all rows. */
span.fnToggle, span.fnTogglePlaceholder
                 { display: inline-block; width: 1em;
                   font-family: monospace; user-select: none;
                   margin-right: 4px; text-align: center; }
span.fnToggle    { cursor: pointer; color: #284fa8; }
span.fnTogglePlaceholder { color: transparent; }
span.fnToggle:focus { outline: 1px dotted #284fa8; }
tr.fileFunctions[hidden] { display: none; }
tr.fileFunctions > td   { background-color: #f4f8ff; padding: 6px 24px; }
/* fnInner fills the parent <td> so the dropdown matches the index
   table's width; table-layout:fixed plus a max-width on the function
   name cell prevents long mangled names from blowing the row open. */
table.fnInner   { border-collapse: collapse; margin: 4px 0;
                  table-layout: fixed; width: 100%; }
table.fnInner td { padding: 1px 6px; font-family: monospace;
                   overflow-wrap: break-word; }
table.fnInner col.fnNameCol  { width: auto; }
table.fnInner col.fnLineCol  { width: 5em; }
table.fnInner col.fnCallsCol { width: 7em; }
table.fnInner col.fnBarCol   { width: 8em; }
table.fnInner col.fnRateCol  { width: 5em; }
table.fnInner col.fnTotalCol { width: 5em; }
table.fnInner col.fnHitCol   { width: 5em; }
table.fnInner td.tableHead { font-family: sans-serif; }

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


@dataclass
class Metric:
    """One summary metric row on a page (Lines, Functions, Branches)."""

    label: str       # "Lines", "Functions", "Branches"
    pct: float
    total: int
    hit: int


def _render_metric_row(
    left_label: str, left_value: str, metric: Metric
) -> str:
    pct_str = f"{metric.pct:.1f}&nbsp;%" if metric.total > 0 else "-"
    tier = _tier(metric.pct) if metric.total > 0 else "Hi"
    return (
        "          <tr>\n"
        f"            <td class=\"headerItem\">{html.escape(left_label)}</td>\n"
        f"            <td class=\"headerValue\">{left_value}</td>\n"
        "            <td></td>\n"
        f"            <td class=\"headerItem\">{metric.label}:</td>\n"
        f"            <td class=\"headerCovTableEntry{tier}\">{pct_str}</td>\n"
        f"            <td class=\"headerCovTableEntry\">{metric.total}</td>\n"
        f"            <td class=\"headerCovTableEntry\">{metric.hit}</td>\n"
        "          </tr>"
    )


def _render_page_header(
    title: str,
    breadcrumb_html: str,
    test_name: str,
    test_date: str,
    metrics: List[Metric],
) -> str:
    # Always show at least the Lines metric. Left-column rows are
    # Test / Date / (blank) — extra rows beyond 2 metrics get a blank
    # left label.
    left_rows = [
        ("Test:", html.escape(test_name)),
        ("Date:", html.escape(test_date)),
    ]
    while len(left_rows) < len(metrics):
        left_rows.append(("", ""))

    rows_html = "\n".join(
        _render_metric_row(left_rows[i][0], left_rows[i][1], metrics[i])
        for i in range(len(metrics))
    )

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
{rows_html}
        </table>
      </td>
    </tr>
    <tr><td class="ruler"></td></tr>
  </table>
"""


INDEX_TOGGLE_SCRIPT = """\
<script>
(function () {
  function toggle(el) {
    var row = el.closest('tr');
    if (!row) return;
    var next = row.nextElementSibling;
    if (!next || !next.classList.contains('fileFunctions')) return;
    var hidden = next.hasAttribute('hidden');
    if (hidden) {
      next.removeAttribute('hidden');
      el.textContent = '\\u25BC';  // ▼
      el.setAttribute('aria-expanded', 'true');
    } else {
      next.setAttribute('hidden', '');
      el.textContent = '\\u25B6';  // ▶
      el.setAttribute('aria-expanded', 'false');
    }
  }
  document.addEventListener('click', function (e) {
    var t = e.target;
    if (t && t.classList && t.classList.contains('fnToggle')) {
      toggle(t);
    }
  });
  document.addEventListener('keydown', function (e) {
    var t = e.target;
    if (t && t.classList && t.classList.contains('fnToggle') &&
        (e.key === 'Enter' || e.key === ' ')) {
      e.preventDefault();
      toggle(t);
    }
  });
})();
</script>
"""


def _render_page_footer(extra_body: str = "") -> str:
    return f"""  <br>
  <table class="chrome" cellpadding="0" cellspacing="0">
    <tr><td class="ruler"></td></tr>
    <tr><td class="versionInfo">Generated by:
      <a href="{GENERATOR_URL}">{GENERATOR_NAME}</a></td></tr>
  </table>
{extra_body}</body>
</html>
"""


def _gradient_color(pct: float) -> str:
    """HSL color for a coverage rate. 0 % → red (hue 0); 100 % →
    green (hue 120). Linear interpolation in hue space gives a
    smooth red → orange → yellow → lime → green ramp without the
    bright-tier banding."""
    pct = max(0.0, min(100.0, pct))
    hue = pct * 1.2  # 0..120
    return f"hsl({hue:.0f}, 70%, 50%)"


def _render_rate_bar(pct: float) -> str:
    fill_w = max(0, min(100, int(round(pct))))
    rest_w = 100 - fill_w
    color = _gradient_color(pct)
    return (
        f'<span class="coverBarOutline">'
        f'<span class="coverBarFill" '
        f'style="background-color:{color};width:{fill_w}px"></span>'
        f'<span class="coverBarRest" style="width:{rest_w}px"></span>'
        f"</span>"
    )


def _overall_metrics(records: List[FileRecord]) -> List[Metric]:
    """Compute the summary metrics to display in the page header.

    Lines is always shown. Functions and Branches appear only if
    the LCOV carries any such records.
    """
    total_l = sum(r.total_lines for r in records)
    hit_l = sum(r.hit_lines for r in records)
    metrics = [
        Metric(
            label="Lines",
            pct=(100.0 * hit_l / total_l) if total_l else 0.0,
            total=total_l,
            hit=hit_l,
        )
    ]
    if has_any_functions(records):
        total_fn = sum(r.total_functions for r in records)
        hit_fn = sum(r.hit_functions for r in records)
        metrics.append(
            Metric(
                label="Functions",
                pct=(100.0 * hit_fn / total_fn) if total_fn else 0.0,
                total=total_fn,
                hit=hit_fn,
            )
        )
    if has_any_branches(records):
        total_br = sum(r.total_branches for r in records)
        hit_br = sum(r.hit_branches for r in records)
        metrics.append(
            Metric(
                label="Branches",
                pct=(100.0 * hit_br / total_br) if total_br else 0.0,
                total=total_br,
                hit=hit_br,
            )
        )
    return metrics


def _index_col_group(
    has_bar: bool,
    rate: float,
    total: int,
    hit: int,
) -> str:
    """Render one metric's cells inside an index row.

    Returns either bar+rate+total+hit (has_bar) or rate+total+hit.
    """
    tier = _tier(rate) if total > 0 else "Hi"
    rate_str = f"{rate:.1f}&nbsp;%" if total > 0 else "-"
    parts = []
    if has_bar:
        parts.append(
            f'              <td class="coverBar" align="center">'
            f'{_render_rate_bar(rate)}</td>'
        )
    parts.append(f'              <td class="coverPer{tier}">{rate_str}</td>')
    parts.append(f'              <td class="coverNumDflt">{total}</td>')
    parts.append(f'              <td class="coverNumDflt">{hit}</td>')
    return "\n".join(parts)


def render_index(
    records: List[FileRecord],
    output_dir: str,
    title: str,
    test_name: str,
    test_date: str,
    prefix: str,
    file_map: Dict[str, str],
) -> None:
    show_fns = has_any_functions(records)
    show_br = has_any_branches(records)

    header = _render_page_header(
        title=title,
        breadcrumb_html="top level",
        test_name=test_name,
        test_date=test_date,
        metrics=_overall_metrics(records),
    )

    # Build the column-header rows. Line group always has the bar
    # (4 cells); function/branch groups omit the bar (3 cells) to
    # keep the table narrow enough to read.
    extra_header_cells = ""
    sub_rate_cells = (
        '        <td class="tableHead" colspan="2">Rate</td>\n'
        '        <td class="tableHead">Total</td>\n'
        '        <td class="tableHead">Hit</td>'
    )
    extra_sub_rate = ""
    cols_for_empty = 5
    if show_fns:
        extra_header_cells += (
            '\n        <td class="tableHead" colspan="3">Function Coverage</td>'
        )
        extra_sub_rate += (
            '\n        <td class="tableHead">Rate</td>'
            '\n        <td class="tableHead">Total</td>'
            '\n        <td class="tableHead">Hit</td>'
        )
        cols_for_empty += 3
    if show_br:
        extra_header_cells += (
            '\n        <td class="tableHead" colspan="3">Branch Coverage</td>'
        )
        extra_sub_rate += (
            '\n        <td class="tableHead">Rate</td>'
            '\n        <td class="tableHead">Total</td>'
            '\n        <td class="tableHead">Hit</td>'
        )
        cols_for_empty += 3

    # Per-file rows. Sort by display-path for determinism. When a file
    # has function records, emit a second hidden <tr> immediately after
    # the summary row holding the function table; a click on the file
    # name's chevron reveals it (handled by inline JS at the bottom of
    # the page; without JS, it stays hidden and per-file pages remain
    # the way to drill in).
    rows_html: List[str] = []
    sorted_records = sorted(records, key=lambda r: _display_path(r.path, prefix))
    for rec in sorted_records:
        display = _display_path(rec.path, prefix)
        href = file_map[rec.path]
        has_fn_table = bool(rec.functions)
        # Leading column: optional expand chevron + the file link.
        if has_fn_table:
            toggle = (
                '<span class="fnToggle" tabindex="0" role="button" '
                f'aria-label="Toggle functions for {html.escape(display)}" '
                'aria-expanded="false">▶</span>'
            )
        else:
            # Placeholder reserves the same column width so file-name
            # columns line up across rows. Hidden via color: transparent.
            toggle = '<span class="fnTogglePlaceholder">▶</span>'
        cells: List[str] = [
            f'              <td class="coverFile">{toggle}'
            f'<a href="{html.escape(href)}">{html.escape(display)}</a></td>',
            _index_col_group(
                has_bar=True,
                rate=rec.percent,
                total=rec.total_lines,
                hit=rec.hit_lines,
            ),
        ]
        if show_fns:
            cells.append(
                _index_col_group(
                    has_bar=False,
                    rate=rec.percent_functions,
                    total=rec.total_functions,
                    hit=rec.hit_functions,
                )
            )
        if show_br:
            cells.append(
                _index_col_group(
                    has_bar=False,
                    rate=rec.percent_branches,
                    total=rec.total_branches,
                    hit=rec.hit_branches,
                )
            )
        rows_html.append(
            '            <tr class="fileSummary">\n'
            + "\n".join(cells)
            + "\n            </tr>"
        )
        if has_fn_table:
            rows_html.append(
                f'            <tr class="fileFunctions" hidden>\n'
                f'              <td colspan="{cols_for_empty}">'
                f"{_render_inline_functions_table(rec, href)}"
                f"</td>\n"
                f"            </tr>"
            )

    if not records:
        rows_html.append(
            f'            <tr><td colspan="{cols_for_empty}" class="footnote" '
            f'style="text-align:center">No coverage data found in input.</td></tr>'
        )

    prefix_note = ""
    if prefix:
        prefix_note = (
            f'        <tr><td colspan="{cols_for_empty}" class="footnote">'
            f"Common path prefix: <code>{html.escape(prefix)}</code></td></tr>\n"
        )

    body = f"""  <center>
    <table class="indexTable" cellpadding="1" cellspacing="1" border="0">
      <tr>
        <td class="tableHead" rowspan="2">File</td>
        <td class="tableHead" colspan="4">Line Coverage</td>{extra_header_cells}
      </tr>
      <tr>
{sub_rate_cells}{extra_sub_rate}
      </tr>
{chr(10).join(rows_html)}
{prefix_note}    </table>
  </center>
"""

    extra_script = INDEX_TOGGLE_SCRIPT if any(r.functions for r in records) else ""
    out = header + body + _render_page_footer(extra_body=extra_script)
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
    metrics = [
        Metric(
            label="Lines",
            pct=record.percent,
            total=record.total_lines,
            hit=record.hit_lines,
        )
    ]
    if record.functions:
        metrics.append(
            Metric(
                label="Functions",
                pct=record.percent_functions,
                total=record.total_functions,
                hit=record.hit_functions,
            )
        )
    if record.branches:
        metrics.append(
            Metric(
                label="Branches",
                pct=record.percent_branches,
                total=record.total_branches,
                hit=record.hit_branches,
            )
        )

    header = _render_page_header(
        title=f"{title} - {display_name}",
        breadcrumb_html=breadcrumb,
        test_name=test_name,
        test_date=test_date,
        metrics=metrics,
    )

    parts: List[str] = []
    # The per-file Functions table moved to the index (expandable per
    # row); the header summary "Functions:" line remains so users still
    # see the file's function coverage rate when they drill in.
    if source_text is not None:
        parts.append(_render_source_view(record, source_text))
    else:
        parts.append(_render_placeholder_view(record))

    body = "\n".join(parts)

    out = header + body + _render_page_footer()
    with open(os.path.join(output_dir, out_filename), "w", encoding="utf-8") as f:
        f.write(out)


def _render_inline_functions_table(record: FileRecord, file_href: str) -> str:
    """Render a Functions table to embed inside an index <td>.

    Columns mirror the file row's layout for the line-coverage half:

      Function name | Line | Calls | Bar | Rate% | Total | Hit

    where "Calls" is the FNDA hit count and the right-hand four cells
    describe the function's *line* coverage (how many DA lines that
    fall inside the function's range were exercised).
    """
    from lcov_io import function_line_coverage

    items = sorted(
        record.functions.items(), key=lambda kv: (kv[1].first_line, kv[0])
    )
    coverage = function_line_coverage(record)

    rows: List[str] = []
    for name, fn in items:
        calls_cls = "coverFnHi" if fn.hits > 0 else "coverFnLo"
        if fn.first_line > 0:
            line_cell = (
                f'<a href="{html.escape(file_href)}#L{fn.first_line}">'
                f"{fn.first_line}</a>"
            )
        else:
            line_cell = "-"

        total, hit = coverage.get(name, (0, 0))
        if total > 0:
            pct = 100.0 * hit / total
            tier = _tier(pct)
            rate_cell = (
                f'<td class="coverPer{tier}">{pct:.1f}&nbsp;%</td>'
            )
            bar_cell = (
                f'<td class="coverBar" align="center">{_render_rate_bar(pct)}</td>'
            )
            total_cell = f'<td class="coverNumDflt">{total}</td>'
            hit_cell = f'<td class="coverNumDflt">{hit}</td>'
        else:
            # No DA lines fall in the function's range — usually
            # because first_line == 0 (orphan FNDA) or the file has
            # no DA records at all. Render dashes.
            rate_cell = '<td class="coverNumDflt">-</td>'
            bar_cell = '<td class="coverBar"></td>'
            total_cell = '<td class="coverNumDflt">-</td>'
            hit_cell = '<td class="coverNumDflt">-</td>'

        rows.append(
            "        <tr>"
            f'<td class="coverFn">{html.escape(name)}</td>'
            f'<td class="coverNumDflt">{line_cell}</td>'
            f'<td class="{calls_cls}">{fn.hits}</td>'
            f"{bar_cell}{rate_cell}{total_cell}{hit_cell}"
            "</tr>"
        )

    return (
        '<table class="fnInner" cellpadding="1" cellspacing="1" border="0">\n'
        '        <colgroup>\n'
        '          <col class="fnNameCol">\n'
        '          <col class="fnLineCol">\n'
        '          <col class="fnCallsCol">\n'
        '          <col class="fnBarCol">\n'
        '          <col class="fnRateCol">\n'
        '          <col class="fnTotalCol">\n'
        '          <col class="fnHitCol">\n'
        '        </colgroup>\n'
        "        <tr>\n"
        '          <td class="tableHead">Function</td>\n'
        '          <td class="tableHead">Line</td>\n'
        '          <td class="tableHead">Calls</td>\n'
        '          <td class="tableHead" colspan="2">Line Coverage</td>\n'
        '          <td class="tableHead">Total</td>\n'
        '          <td class="tableHead">Hit</td>\n'
        "        </tr>\n"
        + "\n".join(rows)
        + "\n      </table>"
    )


BRANCH_COL_WIDTH = 10  # chars, wide enough for "(999/999)" + padding


def _branches_by_line(
    record: FileRecord,
) -> Dict[int, List[Optional[int]]]:
    """Group BRDA records by source line. Preserves (block, branch_id)
    order so tooltip indexing matches LCOV's."""
    result: Dict[int, List[Optional[int]]] = {}
    for (line, block, branch_id), taken in sorted(record.branches.items()):
        result.setdefault(line, []).append(taken)
    return result


def _render_branch_cell(branches_on_line: List[Optional[int]]) -> str:
    """Render the branch-column content for one source line.

    Returns a string exactly BRANCH_COL_WIDTH visual chars wide. When
    no branches exist on this line, returns plain spaces (so it still
    aligns with branched lines below). Otherwise returns a tiered span
    with a title tooltip listing per-branch counts.
    """
    if not branches_on_line:
        return " " * BRANCH_COL_WIDTH

    total = len(branches_on_line)
    hit = sum(1 for t in branches_on_line if t is not None and t > 0)
    if hit == total:
        cls = "branchAll"
    elif hit == 0:
        cls = "branchNone"
    else:
        cls = "branchPart"

    text = f"({hit}/{total})"
    if len(text) > BRANCH_COL_WIDTH:
        text = text[:BRANCH_COL_WIDTH]

    # Per-branch tooltip: "br0: 42; br1: 0; br2: -"
    tt_parts = [
        f"br{i}: {t if t is not None else '-'}"
        for i, t in enumerate(branches_on_line)
    ]
    title = html.escape("; ".join(tt_parts), quote=True)

    pad_after = BRANCH_COL_WIDTH - len(text)
    return f'<span class="{cls}" title="{title}">{text}</span>{" " * pad_after}'


def _render_source_view(record: FileRecord, source_text: str) -> str:
    # Split source into lines; preserve numbering 1-based.
    src_lines = source_text.splitlines()

    has_branches = bool(record.branches)
    branches_by_line = _branches_by_line(record) if has_branches else {}

    parts: List[str] = ["<table><tr><td>"]
    if has_branches:
        # Headings: "Line data" above the 12-char hits column (starts at
        # col 13); "Branch" above the 10-char branch column (cols 23-32);
        # "Source code" above the source body (col 36+).
        heading = (
            "            Line data     Branch    Source code"
        )
    else:
        heading = "            Line data    Source code"
    parts.append(f'<pre class="sourceHeading">{heading}</pre>')
    parts.append('<pre class="source">')

    for idx, raw in enumerate(src_lines, start=1):
        esc = html.escape(raw)
        hits = record.lines.get(idx)
        line_num_cell = f'<span class="lineNum">{idx:>8}</span>'
        if not has_branches:
            # Phase-1 layout, preserved byte-for-byte for files with no
            # branch records.
            if hits is None:
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
        else:
            # Phase-2b layout: insert a 10-char branch column between
            # hits and ` : source`.
            branch_cell = _render_branch_cell(branches_by_line.get(idx, []))
            if hits is None:
                # Non-executable: 14 spaces where covered would have
                # "[sp outside tlaGNC][12-char hits][sp]", then the
                # 10-char branch cell, then ` : source`. Branch cell
                # may still be populated (e.g. for condition lines
                # that carry BRDA but no DA).
                parts.append(
                    f'<span id="L{idx}">{line_num_cell}'
                    f'              {branch_cell} : {esc}</span>'
                )
            elif hits > 0:
                parts.append(
                    f'<span id="L{idx}">{line_num_cell} '
                    f'<span class="tlaGNC">{hits:>12} {branch_cell} : '
                    f"{esc}</span></span>"
                )
            else:
                parts.append(
                    f'<span id="L{idx}">{line_num_cell} '
                    f'<span class="tlaUNC">{0:>12} {branch_cell} : '
                    f"{esc}</span></span>"
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


def has_any_branches(records: List[FileRecord]) -> bool:
    return any(r.branches for r in records)


def has_any_functions(records: List[FileRecord]) -> bool:
    return any(r.functions for r in records)


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
        "--slangc-filter",
        action="store_true",
        help=(
            "Restrict to the slangc compiler-only file set CI uses "
            "(mirrors tools/coverage/slangc-ignore-patterns.sh: drops "
            "external/, build/prelude/, generated FIDDLE / capability "
            "tables, language-server / record-replay / glslang, etc.). "
            "Applied on top of --filter-include / --filter-exclude."
        ),
    )
    p.add_argument(
        "--quiet",
        action="store_true",
        help="Suppress progress output",
    )
    return p


def main(argv: Optional[List[str]] = None) -> int:
    args = build_argparser().parse_args(argv)

    try:
        records = parse_lcov(args.input)
    except LcovParseError as e:
        print(f"slang-coverage-html: {e}", file=sys.stderr)
        return 2

    records = apply_filters(records, args.filter_include, args.filter_exclude)
    if args.slangc_filter:
        before = len(records)
        records = apply_slangc_filter(records)
        if not args.quiet:
            dropped = before - len(records)
            print(
                f"slang-coverage-html: --slangc-filter dropped "
                f"{dropped} file(s); {len(records)} kept",
                file=sys.stderr,
            )
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
