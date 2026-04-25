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
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Optional, Tuple

# Sibling assets — kept on disk so CSS / JS linters can see them.
_ASSET_DIR = Path(__file__).resolve().parent

# Shared LCOV parser / writer / data model. Sibling module so both the
# renderer and the merge tool import the same code.
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from lcov_io import (  # noqa: E402
    AuthSummary,
    FileRecord,
    Function,
    LcovParseError,
    SourceResolver,
    parse_lcov,
    parse_llvm_cov_report,
)

MARKER_NAME = "slang-coverage-html.marker"
GENERATOR_NAME = "slang-coverage-html"
GENERATOR_URL = "https://github.com/shader-slang/slang"

# ---------------------------------------------------------------------------
# Style constants
# ---------------------------------------------------------------------------
#
# Magic numbers and characters that affect rendering live here so a
# style change is one edit, not a grep across the file.

# Rate tier thresholds. Aligned with the gradient watermarks below
# so the categorical tier (Lo/Med/Hi) matches what the bar color
# already communicates: green at 80, warm under 70, red at 0.
TIER_HI = 80.0
TIER_MED = 70.0

# Bar fill geometry. The progress bar is a 100 px outline filled
# proportional to the coverage percent.
BAR_PIXEL_WIDTH = 100

# HSL gradient watermarks. Piecewise-linear interpolation over hue:
#   0 %  → 0   (red)
#   70 % → 30  (red-orange — bottom of the "warm" band)
#   80 % → 90  (yellow-green — bottom of the "good" band)
#   100 %→ 120 (green)
# Choosing watermarks instead of a single linear ramp keeps everything
# below 70 % visually red and pushes the green into the 80+ range.
GRADIENT_HUE_WATERMARKS: Tuple[Tuple[float, float], ...] = (
    (0.0, 0.0),
    (70.0, 30.0),
    (80.0, 90.0),
    (100.0, 120.0),
)
GRADIENT_BAR_SATURATION = "70%"
GRADIENT_BAR_LIGHTNESS = "50%"
GRADIENT_CELL_SATURATION = "60%"
GRADIENT_CELL_LIGHTNESS = "85%"

# Phase-2b branch column. Width in monospace character cells.
BRANCH_COL_WIDTH = 10

# Disclosure widget glyphs.
CHEVRON_OPEN = "\u25BC"   # ▼
CHEVRON_CLOSED = "\u25B6"  # ▶


# ---------------------------------------------------------------------------
# HTML generation
# ---------------------------------------------------------------------------


INLINE_CSS = _ASSET_DIR.joinpath("style.css").read_text(encoding="utf-8")


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


def _render_page_header(
    title: str,
    breadcrumb_html: str,
    test_name: str,
    test_date: str,
    metrics: List[Metric],
    show_metric_grid: bool = True,
) -> str:
    """Top-of-page chrome: title bar, breadcrumb / test / date,
    and (optionally) a transposed metric grid where columns are
    Lines / Functions / Branches and rows are Coverage / Total / Hit.

    `show_metric_grid=False` is the right call on the top-level
    index page: the index table's first dirHeader row already shows
    the same totals, so the chrome grid would be redundant. Per-file
    pages keep the grid (no equivalent dirHeader available).

    The whole block is wrapped in `<div class="topChrome">` with
    `position: sticky; top: 0` so it stays visible when the user
    scrolls down through the file/source list.
    """
    # Metric cards: one flex card per metric. Each card shows the
    # label, the rate %, a thin progress bar, and `hit / total`.
    # Wider and shorter than the old transposed table.
    def _metric_card(m: Metric) -> str:
        if m.total > 0:
            fill_color = _gradient_color(m.pct)
            fill_pct = max(0, min(100, m.pct))
            value_html = f"{m.pct:.1f}&nbsp;%"
            counts_html = f"{m.hit:,} / {m.total:,}"
        else:
            fill_color = "#cfd4d8"
            fill_pct = 0
            value_html = "-"
            counts_html = "-"
        return (
            f'    <div class="metricCard">\n'
            f'      <span class="metricLabel">{html.escape(m.label)}</span>\n'
            f'      <span class="metricValue">{value_html}</span>\n'
            f'      <div class="metricBarMini">'
            f'<div class="metricBarFill" style="width:{fill_pct:.1f}%;'
            f'background-color:{fill_color}"></div></div>\n'
            f'      <span class="metricCounts">{counts_html}</span>\n'
            f"    </div>"
        )

    cards_html = "\n".join(_metric_card(m) for m in metrics)

    metric_grid_html = (
        f"""  <div class="metricCards">
{cards_html}
  </div>"""
        if show_metric_grid
        else ""
    )

    return f"""<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html lang="en">
<head>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
  <title>{html.escape(title)}</title>
  <style>{INLINE_CSS}</style>
</head>
<body>
<div class="topChrome">
  <table class="chrome" cellpadding="0" cellspacing="0">
    <tr><td class="title">{html.escape(title)}</td></tr>
    <tr><td class="ruler"></td></tr>
  </table>
  <table class="chromeMeta">
    <tr>
      <td class="headerItem">Current view:</td>
      <td class="headerValue">{breadcrumb_html}</td>
      <td class="headerItem">Test:</td>
      <td class="headerValue">{html.escape(test_name)}</td>
      <td class="headerItem">Date:</td>
      <td class="headerValue">{html.escape(test_date)}</td>
    </tr>
  </table>
{metric_grid_html}
  <table class="chrome" cellpadding="0" cellspacing="0">
    <tr><td class="ruler"></td></tr>
  </table>
</div>
"""


_INLINE_JS = _ASSET_DIR.joinpath("script.js").read_text(encoding="utf-8")
CHROME_MEASURE_SCRIPT = "<script>\n" + _INLINE_JS + "</script>\n"


# INDEX_TOGGLE_SCRIPT historically wrapped two IIFEs (chrome-height
# measure + dir/file toggle handlers); they're both in script.js now.
INDEX_TOGGLE_SCRIPT = CHROME_MEASURE_SCRIPT


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


def _hue_for_pct(pct: float) -> float:
    """Piecewise-linear hue from coverage percentage; see
    `GRADIENT_HUE_WATERMARKS` for the breakpoints."""
    p = max(0.0, min(100.0, pct))
    for (p0, h0), (p1, h1) in zip(
        GRADIENT_HUE_WATERMARKS, GRADIENT_HUE_WATERMARKS[1:]
    ):
        if p <= p1:
            if p1 == p0:
                return h0
            return h0 + (p - p0) * (h1 - h0) / (p1 - p0)
    return GRADIENT_HUE_WATERMARKS[-1][1]


def _gradient_color(pct: float) -> str:
    """HSL color for a coverage rate — the *bar fill*: vivid,
    saturated, fully visible on a small fixed-size element."""
    return (
        f"hsl({_hue_for_pct(pct):.0f}, "
        f"{GRADIENT_BAR_SATURATION}, {GRADIENT_BAR_LIGHTNESS})"
    )


def _gradient_cell_bg(pct: float) -> str:
    """Soft gradient background for a percent-rate cell — same hue
    ramp as `_gradient_color`, at high lightness so dark body text
    stays legible on top."""
    return (
        f"hsl({_hue_for_pct(pct):.0f}, "
        f"{GRADIENT_CELL_SATURATION}, {GRADIENT_CELL_LIGHTNESS})"
    )


def _rate_cell(pct: float, total: int) -> str:
    """Standard percent-rate <td> with the gradient background."""
    if total <= 0:
        return '<td class="coverNumDflt">-</td>'
    color = _gradient_cell_bg(pct)
    return (
        f'<td class="coverPerCell" style="background-color:{color}">'
        f"{pct:.1f}&nbsp;%</td>"
    )


def _render_rate_bar(pct: float) -> str:
    fill_w = max(0, min(BAR_PIXEL_WIDTH, int(round(pct * BAR_PIXEL_WIDTH / 100))))
    rest_w = BAR_PIXEL_WIDTH - fill_w
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
    parts = []
    if has_bar:
        parts.append(
            f'              <td class="coverBar" align="center">'
            f'{_render_rate_bar(rate)}</td>'
        )
    parts.append(f"              {_rate_cell(rate, total)}")
    parts.append(f'              <td class="coverNumDflt">{total}</td>')
    parts.append(f'              <td class="coverNumDflt">{hit}</td>')
    return "\n".join(parts)


def _index_dir_tree(
    records: List[FileRecord], prefix: str
) -> Tuple["collections.OrderedDict[str, List[FileRecord]]", List[str]]:
    """Group records by their parent directory (post-prefix-strip)
    and return the lexicographically-sorted set of all directory
    paths including ancestors. Walking that list in order is a
    natural DFS — each `source/`, `source/slang/`,
    `source/slang/optimizer/`, etc. surfaces in order so nested
    rows can be emitted with one pass."""
    sorted_records = sorted(records, key=lambda r: _display_path(r.path, prefix))
    files_by_dir: "collections.OrderedDict[str, List[FileRecord]]" = (
        collections.OrderedDict()
    )
    for rec in sorted_records:
        display = _display_path(rec.path, prefix)
        if "/" in display:
            dirpath, _, _ = display.rpartition("/")
        else:
            dirpath = ""
        files_by_dir.setdefault(dirpath, []).append(rec)

    all_dirs = set()
    for dirpath in files_by_dir:
        parts = dirpath.split("/") if dirpath else []
        for i in range(len(parts) + 1):
            all_dirs.add("/".join(parts[:i]))
    return files_by_dir, sorted(all_dirs)


def _files_under(
    files_by_dir: Dict[str, List[FileRecord]], target: str
) -> List[FileRecord]:
    """All FileRecords whose dirpath equals or descends from `target`."""
    if target == "":
        return [f for fs in files_by_dir.values() for f in fs]
    return [
        f
        for d, fs in files_by_dir.items()
        for f in fs
        if d == target or d.startswith(target + "/")
    ]


def _dir_depth(dirpath: str) -> int:
    return 0 if dirpath == "" else dirpath.count("/") + 1


def _indent_style(depth: int) -> str:
    """Per-row left padding so the tree shape is visible even when
    row backgrounds are uniform."""
    return f"padding-left: calc(10px + {1.4 * depth:.2f}em);"


def _index_thead_html(
    show_fns: bool, show_br: bool
) -> Tuple[str, str, int]:
    """Returns (top-row extra cells, sub-rate-row extra cells, total
    columns). The line group always has 4 cells (Bar+Rate+Total+Hit);
    fns / branches groups have 3 cells each (no bar) when present."""
    extra_top = ""
    extra_sub = ""
    cols_total = 5
    if show_fns:
        extra_top += '\n        <td class="tableHead" colspan="3">Function Coverage</td>'
        extra_sub += (
            '\n        <td class="tableHead">Rate</td>'
            '\n        <td class="tableHead">Total</td>'
            '\n        <td class="tableHead">Hit</td>'
        )
        cols_total += 3
    if show_br:
        extra_top += '\n        <td class="tableHead" colspan="3">Branch Coverage</td>'
        extra_sub += (
            '\n        <td class="tableHead">Rate</td>'
            '\n        <td class="tableHead">Total</td>'
            '\n        <td class="tableHead">Hit</td>'
        )
        cols_total += 3
    return extra_top, extra_sub, cols_total


def _index_colgroup_html(show_fns: bool, show_br: bool) -> str:
    """Colgroup widths driven by CSS classes; defined here once and
    reused by `fnInner` so per-function cells line up under their
    parent column."""
    cols = [
        '        <col class="colFile">',
        '        <col class="colLBar">',
        '        <col class="colLRate">',
        '        <col class="colLTotal">',
        '        <col class="colLHit">',
    ]
    if show_fns:
        cols += [
            '        <col class="colFRate">',
            '        <col class="colFTotal">',
            '        <col class="colFHit">',
        ]
    if show_br:
        cols += [
            '        <col class="colBRate">',
            '        <col class="colBTotal">',
            '        <col class="colBHit">',
        ]
    return "      <colgroup>\n" + "\n".join(cols) + "\n      </colgroup>"


def _render_dir_header_row(
    dirpath: str,
    dir_records: List[FileRecord],
    show_fns: bool,
    show_br: bool,
) -> str:
    depth = _dir_depth(dirpath)
    label = (dirpath.rsplit("/", 1)[-1] + "/") if dirpath else "(top level)"
    file_count = len(dir_records)
    file_count_label = f"{file_count} file{'s' if file_count != 1 else ''}"

    # Aggregated stats over this dir + all descendants.
    t_l = sum(r.total_lines for r in dir_records)
    h_l = sum(r.hit_lines for r in dir_records)
    rate_l = (100.0 * h_l / t_l) if t_l else 0.0
    t_fn = sum(r.total_functions for r in dir_records)
    h_fn = sum(r.hit_functions for r in dir_records)
    rate_fn = (100.0 * h_fn / t_fn) if t_fn else 0.0
    t_br = sum(r.total_branches for r in dir_records)
    h_br = sum(r.hit_branches for r in dir_records)
    rate_br = (100.0 * h_br / t_br) if t_br else 0.0

    dir_attr = html.escape(dirpath)
    cells: List[str] = [
        f'              <td class="coverDirectory" style="{_indent_style(depth)}">'
        f'<span class="dirToggle" tabindex="0" role="button" '
        f'aria-expanded="true" data-path="{dir_attr}" '
        f'aria-label="Toggle directory {html.escape(label)}">'
        f"{CHEVRON_OPEN}</span>{html.escape(label)} "
        f'<span style="opacity:.7;font-weight:400">({file_count_label})</span>'
        f"</td>",
        _index_col_group(has_bar=True, rate=rate_l, total=t_l, hit=h_l),
    ]
    if show_fns:
        cells.append(_index_col_group(has_bar=False, rate=rate_fn, total=t_fn, hit=h_fn))
    if show_br:
        cells.append(_index_col_group(has_bar=False, rate=rate_br, total=t_br, hit=h_br))
    return (
        f'            <tr class="dirHeader" data-path="{dir_attr}" '
        f'data-depth="{depth}">\n'
        + "\n".join(cells)
        + "\n            </tr>"
    )


def _render_file_row(
    rec: FileRecord,
    prefix: str,
    file_map: Dict[str, str],
    dir_attr: str,
    file_depth: int,
    show_fns: bool,
    show_br: bool,
) -> str:
    display = _display_path(rec.path, prefix)
    display_basename = display.rsplit("/", 1)[-1]
    href = file_map[rec.path]
    if rec.functions:
        toggle = (
            '<span class="fnToggle" tabindex="0" role="button" '
            f'aria-label="Toggle functions for {html.escape(display)}" '
            f'aria-expanded="false">{CHEVRON_CLOSED}</span>'
        )
    else:
        toggle = f'<span class="fnTogglePlaceholder">{CHEVRON_CLOSED}</span>'
    cells: List[str] = [
        f'              <td class="coverFile" style="{_indent_style(file_depth)}">'
        f"{toggle}"
        f'<a href="{html.escape(href)}">{html.escape(display_basename)}</a></td>',
        _index_col_group(
            has_bar=True, rate=rec.percent, total=rec.total_lines, hit=rec.hit_lines
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
    return (
        f'            <tr class="fileSummary" data-dir="{dir_attr}" '
        f'data-depth="{file_depth}">\n'
        + "\n".join(cells)
        + "\n            </tr>"
    )


def _render_file_functions_row(
    rec: FileRecord,
    file_href: str,
    dir_attr: str,
    file_depth: int,
    show_fns: bool,
    show_br: bool,
    cols_for_empty: int,
) -> Tuple[str, str, str]:
    """Returns (template_id, template_html, hidden_row_html). The
    template carries the fnInner table; the hidden row references it
    via `data-fn-tmpl` and gets cloned in on first chevron click."""
    tmpl_id = "fn_" + hashlib.sha1(rec.path.encode("utf-8")).hexdigest()[:10]
    template_html = _render_inline_functions_table(
        rec, file_href, show_fns=show_fns, show_br=show_br
    )
    row_html = (
        f'            <tr class="fileFunctions" data-dir="{dir_attr}" '
        f'data-depth="{file_depth}" data-fn-tmpl="{tmpl_id}" hidden>\n'
        f'              <td colspan="{cols_for_empty}"></td>\n'
        f"            </tr>"
    )
    return tmpl_id, template_html, row_html


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
        # The first dirHeader row already shows aggregated totals;
        # the chrome metric grid would be redundant on the index.
        show_metric_grid=False,
    )

    extra_top, extra_sub, cols_for_empty = _index_thead_html(show_fns, show_br)
    files_by_dir, sorted_all_dirs = _index_dir_tree(records, prefix)

    rows_html: List[str] = []
    fn_templates: List[Tuple[str, str]] = []
    for dirpath in sorted_all_dirs:
        dir_records = _files_under(files_by_dir, dirpath)
        if not dir_records:
            continue
        rows_html.append(
            _render_dir_header_row(dirpath, dir_records, show_fns, show_br)
        )
        dir_attr = html.escape(dirpath)
        file_depth = _dir_depth(dirpath) + 1
        for rec in files_by_dir.get(dirpath, []):
            rows_html.append(
                _render_file_row(
                    rec, prefix, file_map, dir_attr, file_depth, show_fns, show_br
                )
            )
            if rec.functions:
                tid, thtml, hidden_row = _render_file_functions_row(
                    rec, file_map[rec.path], dir_attr, file_depth,
                    show_fns, show_br, cols_for_empty,
                )
                fn_templates.append((tid, thtml))
                rows_html.append(hidden_row)

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

    # Out-of-table <template>s for each file's expanded function
    # listing. Browsers parse <template> children as a DocumentFragment
    # without painting them, so initial DOM stays small even when we
    # have thousands of functions across hundreds of files.
    templates_html = "\n".join(
        f'<template id="{tid}">{thtml}</template>'
        for tid, thtml in fn_templates
    )

    body = f"""  <table class="indexTable" cellpadding="1" cellspacing="1" border="0">
{_index_colgroup_html(show_fns, show_br)}
    <thead>
      <tr>
        <td class="tableHead" rowspan="2">File</td>
        <td class="tableHead" colspan="4">Line Coverage</td>{extra_top}
      </tr>
      <tr>
        <td class="tableHead" colspan="2">Rate</td>
        <td class="tableHead">Total</td>
        <td class="tableHead">Hit</td>{extra_sub}
      </tr>
    </thead>
    <tbody>
{chr(10).join(rows_html)}
{prefix_note}    </tbody>
  </table>
{templates_html}
"""

    out = header + body + _render_page_footer(extra_body=INDEX_TOGGLE_SCRIPT)
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

    # Per-file pages also need the chrome-height measurement script
    # so the sticky source heading sits directly under the chrome.
    out = header + body + _render_page_footer(extra_body=CHROME_MEASURE_SCRIPT)
    with open(os.path.join(output_dir, out_filename), "w", encoding="utf-8") as f:
        f.write(out)


def _fn_line_coverage_cells(l_total: int, l_hit: int) -> str:
    """Bar | Rate | Total | Hit cells for a single function's line
    coverage. Renders dashes when the function has no DA lines in
    its range (orphan FN or untracked body)."""
    if l_total > 0:
        pct = 100.0 * l_hit / l_total
        bar = (
            f'<td class="coverBar" align="center">{_render_rate_bar(pct)}</td>'
        )
        return (
            bar
            + _rate_cell(pct, l_total)
            + f'<td class="coverNumDflt">{l_total}</td>'
            + f'<td class="coverNumDflt">{l_hit}</td>'
        )
    return (
        '<td class="coverBar"></td>'
        '<td class="coverNumDflt">-</td>'
        '<td class="coverNumDflt">-</td>'
        '<td class="coverNumDflt">-</td>'
    )


def _fn_function_coverage_cells(is_hit: bool) -> str:
    """Per-function function-coverage cells (Rate / Total / Hit).
    "Effective" hit semantics — see FileRecord._effective_fn_hit."""
    pct = 100.0 if is_hit else 0.0
    return (
        _rate_cell(pct, 1)
        + '<td class="coverNumDflt">1</td>'
        + f'<td class="coverNumDflt">{1 if is_hit else 0}</td>'
    )


def _fn_branch_coverage_cells(b_total: int, b_hit: int) -> str:
    """Per-function branch-coverage cells (Rate / Total / Hit)."""
    if b_total > 0:
        pct = 100.0 * b_hit / b_total
        return (
            _rate_cell(pct, b_total)
            + f'<td class="coverNumDflt">{b_total}</td>'
            + f'<td class="coverNumDflt">{b_hit}</td>'
        )
    return (
        '<td class="coverNumDflt">-</td>'
        '<td class="coverNumDflt">-</td>'
        '<td class="coverNumDflt">-</td>'
    )


def _fn_inner_colgroup(show_fns: bool, show_br: bool) -> str:
    """Colgroup for fnInner: subdivides parent's File column into
    Name + Line, then reuses the parent's colgroup classes for the
    metric cells (so column widths line up vertically with the
    file-summary row above)."""
    cols = [
        '          <col class="fnNameCol">',
        '          <col class="fnLineCol">',
        '          <col class="colLBar">',
        '          <col class="colLRate">',
        '          <col class="colLTotal">',
        '          <col class="colLHit">',
    ]
    if show_fns:
        cols += [
            '          <col class="colFRate">',
            '          <col class="colFTotal">',
            '          <col class="colFHit">',
        ]
    if show_br:
        cols += [
            '          <col class="colBRate">',
            '          <col class="colBTotal">',
            '          <col class="colBHit">',
        ]
    return "        <colgroup>\n" + "\n".join(cols) + "\n        </colgroup>"


def _fn_inner_thead(show_fns: bool, show_br: bool) -> str:
    extra = ""
    if show_fns:
        extra += '          <td class="tableHead" colspan="3">Function Coverage</td>\n'
    if show_br:
        extra += '          <td class="tableHead" colspan="3">Branch Coverage</td>\n'
    return (
        "        <tr>\n"
        '          <td class="tableHead">Function</td>\n'
        '          <td class="tableHead">Line</td>\n'
        '          <td class="tableHead" colspan="2">Line Coverage</td>\n'
        '          <td class="tableHead">Total</td>\n'
        '          <td class="tableHead">Hit</td>\n'
        + extra
        + "        </tr>"
    )


def _render_inline_functions_table(
    record: FileRecord,
    file_href: str,
    show_fns: bool = False,
    show_br: bool = False,
) -> str:
    """Render the per-file Functions table embedded in an index
    `<td>` (lazy-loaded via <template>).

    Each row carries: Function | Line | LineBar | LineRate |
    LineTotal | LineHit | (FRate | FTotal | FHit) | (BRate | BTotal
    | BHit). Cells line up vertically with the parent indexTable's
    columns of the same names — colgroup widths shared via the
    `colLBar`, `colFRate`, … classes.
    """
    from lcov_io import function_line_coverage, function_branch_coverage

    items = sorted(
        record.functions.items(), key=lambda kv: (kv[1].first_line, kv[0])
    )
    line_cov = function_line_coverage(record)
    br_cov = function_branch_coverage(record)
    eff_hit = record._effective_fn_hit()

    rows: List[str] = []
    for name, fn in items:
        if fn.first_line > 0:
            line_cell = (
                f'<a href="{html.escape(file_href)}#L{fn.first_line}">'
                f"{fn.first_line}</a>"
            )
        else:
            line_cell = "-"

        l_total, l_hit = line_cov.get(name, (0, 0))
        line_cells = _fn_line_coverage_cells(l_total, l_hit)
        fn_cells = (
            _fn_function_coverage_cells(eff_hit.get(name, fn.hits > 0))
            if show_fns
            else ""
        )
        if show_br:
            b_total, b_hit = br_cov.get(name, (0, 0))
            br_cells = _fn_branch_coverage_cells(b_total, b_hit)
        else:
            br_cells = ""

        rows.append(
            "        <tr>"
            f'<td class="coverFn">{html.escape(name)}</td>'
            f'<td class="coverNumDflt">{line_cell}</td>'
            f"{line_cells}{fn_cells}{br_cells}"
            "</tr>"
        )

    return (
        '<table class="fnInner" cellpadding="1" cellspacing="1" border="0">\n'
        + _fn_inner_colgroup(show_fns, show_br)
        + "\n"
        + _fn_inner_thead(show_fns, show_br)
        + "\n"
        + "\n".join(rows)
        + "\n      </table>"
    )


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

    # Heading positions match the data layout exactly, so the column
    # labels sit directly above their data:
    #   no branches:
    #     "    Line"(1-8) " "(9) "        Hits"(10-21) "   "(22-24) Source(25+)
    #   with branches:
    #     "    Line"(1-8) " "(9) "        Hits"(10-21) " "(22)
    #     "  Branch  "(23-32) " : "(33-35) Source(36+)
    #
    # The heading is rendered as a sibling <pre> before <pre class="source">
    # (no surrounding table) so it can be `position: sticky` underneath
    # the page chrome.
    if has_branches:
        heading = "    Line        Hits   Branch    Source code"
    else:
        heading = "    Line        Hits    Source code"

    parts: List[str] = [
        f'<pre class="sourceHeading">{heading}</pre>',
        '<pre class="source">',
    ]

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
    parts.append("</pre>")
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
    include_regex: Optional[List[str]] = None,
    exclude_regex: Optional[List[str]] = None,
) -> List[FileRecord]:
    """Apply include / exclude filters to records.

    Globs (`includes` / `excludes`) match the full repo-relative path
    via `fnmatch`. Regexes (`include_regex` / `exclude_regex`) match
    anywhere in the path via `re.search`. All four flag families
    compose as: include = glob-include AND regex-include; exclude =
    glob-exclude OR regex-exclude.
    """
    inc_re = (
        re.compile("|".join(include_regex)) if include_regex else None
    )
    exc_re = (
        re.compile("|".join(exclude_regex)) if exclude_regex else None
    )
    if not includes and not excludes and not inc_re and not exc_re:
        return records
    out: List[FileRecord] = []
    for r in records:
        p = r.path.replace("\\", "/")
        if includes and not any(fnmatch.fnmatch(p, pat) for pat in includes):
            continue
        if inc_re and not inc_re.search(p):
            continue
        if excludes and any(fnmatch.fnmatch(p, pat) for pat in excludes):
            continue
        if exc_re and exc_re.search(p):
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
    # Compare reported LF/LH against the raw LCOV counts (independent
    # of any auth_override that the renderer may have applied later).
    for r in records:
        derived_lf = len(r.lines)
        derived_lh = sum(1 for h in r.lines.values() if h > 0)
        if r.reported_lf is not None and r.reported_lf != derived_lf:
            print(
                f"slang-coverage-html: warning: {r.path}: LF reported "
                f"{r.reported_lf} but saw {derived_lf} DA lines",
                file=sys.stderr,
            )
        if r.reported_lh is not None and r.reported_lh != derived_lh:
            print(
                f"slang-coverage-html: warning: {r.path}: LH reported "
                f"{r.reported_lh} but saw {derived_lh} hit DA lines",
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
        "--filter-include-regex",
        action="append",
        default=[],
        metavar="REGEX",
        help=(
            "Include-only Python regex (repeatable; matched anywhere "
            "in the SF: path). Composed with --filter-include via AND."
        ),
    )
    p.add_argument(
        "--filter-exclude-regex",
        action="append",
        default=[],
        metavar="REGEX",
        help=(
            "Exclude Python regex (repeatable; matched anywhere in "
            "the SF: path). Composed with --filter-exclude via OR."
        ),
    )
    p.add_argument(
        "--auth-summary",
        default=None,
        metavar="REPORT_TXT",
        help=(
            "Path to an `llvm-cov report` text dump. When supplied, "
            "per-file Lines/Functions/Branches totals on the index, "
            "directory aggregates and per-file pages are taken from "
            "the report instead of from the LCOV — these are the "
            "numbers CI's coverage dashboard quotes. Source-view "
            "rendering (per-line hits, branch markers, function table) "
            "still uses LCOV detail. Files in the LCOV but missing "
            "from the report fall back to LCOV-derived totals."
        ),
    )
    p.add_argument(
        "--quiet",
        action="store_true",
        help="Suppress progress output",
    )
    return p


def apply_auth_summary(
    records: List[FileRecord], auth: AuthSummary, quiet: bool = False
) -> Tuple[int, int]:
    """Attach per-file auth_override to each matching record.

    Returns (matched, unmatched) for caller logging. Match is by
    exact path equality — both the records and the report use repo-
    relative forward-slash paths in the typical CI workflow.
    """
    matched = 0
    unmatched = 0
    for r in records:
        f = auth.get(r.path)
        if f is None:
            unmatched += 1
            continue
        r.auth_override = f
        matched += 1
    return matched, unmatched


def main(argv: Optional[List[str]] = None) -> int:
    args = build_argparser().parse_args(argv)

    try:
        records = parse_lcov(args.input)
    except LcovParseError as e:
        print(f"slang-coverage-html: {e}", file=sys.stderr)
        return 2

    records = apply_filters(
        records,
        args.filter_include,
        args.filter_exclude,
        include_regex=args.filter_include_regex,
        exclude_regex=args.filter_exclude_regex,
    )
    if args.auth_summary:
        try:
            auth = parse_llvm_cov_report(args.auth_summary)
        except LcovParseError as e:
            print(f"slang-coverage-html: {e}", file=sys.stderr)
            return 2
        matched, unmatched = apply_auth_summary(records, auth, quiet=args.quiet)
        if not args.quiet:
            print(
                f"slang-coverage-html: --auth-summary applied to "
                f"{matched} of {len(records)} record(s); "
                f"{unmatched} fell back to LCOV totals",
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
