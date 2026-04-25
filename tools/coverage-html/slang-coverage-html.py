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
/* Palette pulled from shader-slang.org:
   primary teal #105f65, accent orange #F14D1B, success green #2c882c,
   amber #ffc107, red #dc3545, body bg #fff, subtle gray #f8f9fa.
   Tier cells use the categorical brand colors; rate bars use a
   continuous HSL gradient (set per-row inline). */
:root {
  --slang-teal:   #105f65;
  --slang-teal-hover: #0d4d52;
  --slang-orange: #F14D1B;
  --tier-hi:      #2c882c;
  --tier-hi-bg:   #d6ebd5;
  --tier-med:     #ffc107;
  --tier-med-bg:  #fff3cd;
  --tier-lo:      #dc3545;
  --tier-lo-bg:   #f8d7da;
  --row-bg:       #f8f9fa;
  --row-bg-alt:   #ffffff;
  --row-hover:    #e9ecef;
  --text:         #212529;
  --text-muted:   #6c757d;
  --link:         #105f65;
}

body { color: var(--text); background-color: #fff;
       font-family: -apple-system, BlinkMacSystemFont, "Segoe UI",
                    Roboto, Helvetica, Arial, sans-serif;
       margin: 0; padding: 0 24px; }
a:link, a:visited { color: var(--link); text-decoration: underline; }
a:hover           { color: var(--slang-teal-hover); }
a:active          { color: var(--slang-orange); }

table.chrome { width: 100%; border-collapse: collapse; }
td.title   { text-align: center; padding: 14px 0 10px; font-size: 22pt;
             font-weight: 600; color: var(--slang-teal);
             letter-spacing: 0.02em; }
td.ruler   { background-color: var(--slang-teal); height: 3px; padding: 0; }

/* Sticky page chrome: title + meta + transposed metric grid stay
   visible when the user scrolls. The data table's <thead> sticks
   directly underneath at top: var(--chrome-h), measured by JS on
   page load. */
div.topChrome { position: sticky; top: 0; z-index: 20;
                background-color: #fff;
                box-shadow: 0 2px 4px rgba(0, 0, 0, 0.08); }
table.chromeMeta { width: 100%; border-collapse: collapse;
                   margin: 6px 0; }
table.chromeMeta td { padding: 2px 8px; }
table.metricGrid { margin: 8px auto 12px;
                   border-collapse: collapse; }
table.metricGrid td { padding: 4px 14px; text-align: center;
                      min-width: 6em; }
td.metricColHead { background-color: var(--slang-teal); color: #fff;
                   font-weight: 600; }
td.metricRowLabel { background-color: var(--row-bg); color: var(--text-muted);
                    font-weight: 600; text-align: right; }

td.headerItem        { text-align: right; padding-right: 6px; font-weight: 600;
                       vertical-align: top; white-space: nowrap; color: var(--text-muted); }
td.headerValue       { text-align: left; color: var(--slang-teal); font-weight: 600;
                       white-space: nowrap; }
td.headerCovTableHead { text-align: center; padding: 0 6px; white-space: nowrap;
                        color: var(--text-muted); }
td.headerCovTableEntry    { text-align: right; color: var(--text); font-weight: 600;
                            padding-left: 12px; padding-right: 4px;
                            background-color: var(--row-bg); white-space: nowrap; }
td.headerCovTableEntryHi  { text-align: right; color: var(--text); font-weight: 700;
                            padding-left: 12px; padding-right: 4px;
                            background-color: var(--tier-hi-bg); white-space: nowrap; }
td.headerCovTableEntryMed { text-align: right; color: var(--text); font-weight: 700;
                            padding-left: 12px; padding-right: 4px;
                            background-color: var(--tier-med-bg); white-space: nowrap; }
td.headerCovTableEntryLo  { text-align: right; color: var(--text); font-weight: 700;
                            padding-left: 12px; padding-right: 4px;
                            background-color: var(--tier-lo-bg); white-space: nowrap; }

/* Top-level index table: full window width with a small breathing
   margin. Both the file-summary table and any embedded fnInner
   share these column widths so the same metric column lines up
   vertically across the file row and its expanded function rows. */
table.indexTable { margin: 0 auto; width: 100%;
                   border-collapse: collapse; }
table.indexTable col.colFile   { width: 47%; }
table.indexTable col.colLBar   { width: 9%; }
table.indexTable col.colLRate  { width: 5%; }
table.indexTable col.colLTotal { width: 4%; }
table.indexTable col.colLHit   { width: 4%; }
table.indexTable col.colFRate  { width: 5%; }
table.indexTable col.colFTotal { width: 4%; }
table.indexTable col.colFHit   { width: 4%; }
table.indexTable col.colBRate  { width: 5%; }
table.indexTable col.colBTotal { width: 4%; }
table.indexTable col.colBHit   { width: 4%; }

td.tableHead { text-align: center; color: #fff; background-color: var(--slang-teal);
               font-size: 110%; font-weight: 600; padding: 4px 6px;
               white-space: nowrap; letter-spacing: 0.02em; }

/* Make the index table's <thead> sticky right under the chrome.
   `--chrome-h` is set by the inline JS at page load to the
   measured height of `div.topChrome` so the two sticky layers
   stack without overlap. */
table.indexTable thead    { position: sticky;
                            top: var(--chrome-h, 200px);
                            z-index: 10; }
table.indexTable thead td { background-color: var(--slang-teal); }

tr.fileSummary > td:not(.coverBar):not(.coverFile):not(.coverDirectory) {
  text-align: right;
}
tr.fileSummary:nth-child(odd)  > td.coverFile,
tr.fileSummary:nth-child(odd)  > td.coverNumDflt { background-color: var(--row-bg); }

td.coverFile       { text-align: left; padding: 3px 20px 3px 10px;
                     color: var(--link); background-color: var(--row-bg-alt);
                     font-family: ui-monospace, SFMono-Regular, Menlo,
                                  Monaco, Consolas, monospace;
                     overflow-wrap: anywhere; }
td.coverDirectory  { text-align: left; padding: 4px 20px 4px 10px;
                     color: #fff; background-color: var(--slang-teal);
                     font-family: ui-monospace, SFMono-Regular, Menlo,
                                  Monaco, Consolas, monospace;
                     font-weight: 600; }
td.coverBar        { padding: 3px 10px; background-color: var(--row-bg-alt); }
tr.fileSummary:nth-child(odd) > td.coverBar { background-color: var(--row-bg); }
span.coverBarOutline { display: inline-block; height: 10px; width: 100px;
                       background-color: #cfd4d8; vertical-align: middle;
                       line-height: 0; border-radius: 1px; overflow: hidden; }
span.coverBarFill  { display: inline-block; height: 10px; vertical-align: top; }
span.coverBarRest    { background-color: transparent; display: inline-block;
                       height: 10px; vertical-align: top; }

td.coverPerHi   { text-align: right; padding: 3px 10px;
                  background-color: var(--tier-hi-bg);  color: var(--text);
                  font-weight: 700; }
td.coverPerMed  { text-align: right; padding: 3px 10px;
                  background-color: var(--tier-med-bg); color: var(--text);
                  font-weight: 700; }
td.coverPerLo   { text-align: right; padding: 3px 10px;
                  background-color: var(--tier-lo-bg);  color: var(--text);
                  font-weight: 700; }
td.coverNumDflt { text-align: right; padding: 3px 10px;
                  background-color: var(--row-bg-alt); white-space: nowrap; }

td.footnote    { padding: 6px 10px; background-color: var(--row-bg);
                 color: var(--text-muted); font-style: italic; font-size: 85%; }
td.versionInfo { text-align: center; padding: 8px; color: var(--text-muted);
                 font-size: 90%; }

pre.sourceHeading { font-family: ui-monospace, SFMono-Regular, Menlo,
                                 Monaco, Consolas, monospace;
                    font-weight: 700; margin: 0; color: var(--text-muted); }
pre.source        { font-family: ui-monospace, SFMono-Regular, Menlo,
                                 Monaco, Consolas, monospace;
                    margin-top: 2px; }
span.lineNum      { background-color: #f1f3f5; display: inline-block;
                    min-width: 6em; text-align: right; padding-right: 4px;
                    color: var(--text-muted); }
span.tlaGNC       { background-color: #d6ebd5; }
span.tlaUNC       { background-color: #f8d7da; }

td.coverFn        { text-align: left; padding: 2px 20px 2px 10px;
                    color: var(--text);
                    background-color: var(--row-bg-alt);
                    font-family: ui-monospace, SFMono-Regular, Menlo,
                                 Monaco, Consolas, monospace;
                    word-break: break-all; font-size: 92%; }
td.coverFnHi      { text-align: right; padding: 2px 10px;
                    background-color: var(--tier-hi-bg); color: var(--text);
                    font-weight: 600; white-space: nowrap; }
td.coverFnLo      { text-align: right; padding: 2px 10px;
                    background-color: var(--tier-lo-bg); color: var(--text);
                    font-weight: 600; white-space: nowrap; }

span.branchAll   { background-color: var(--tier-hi-bg);  color: var(--text); }
span.branchPart  { background-color: var(--tier-med-bg); color: var(--text); }
span.branchNone  { background-color: var(--tier-lo-bg);  color: var(--text); }

/* Expand-chevrons. The placeholder variant reserves the same column
   width on rows that have no functions, so the file-name column
   stays vertically aligned across all rows. */
span.fnToggle, span.fnTogglePlaceholder, span.dirToggle
                 { display: inline-block; width: 1em;
                   font-family: ui-monospace, SFMono-Regular, Menlo,
                                Monaco, Consolas, monospace;
                   user-select: none; margin-right: 4px; text-align: center; }
span.fnToggle    { cursor: pointer; color: var(--slang-teal); }
span.dirToggle   { cursor: pointer; color: #fff; }
span.fnTogglePlaceholder { color: transparent; }
span.fnToggle:focus, span.dirToggle:focus { outline: 1px dotted var(--slang-orange); }
tr.fileFunctions[hidden]  { display: none; }
tr.fileSummary[hidden]    { display: none; }
/* fileFunctions row's <td> matches the parent indexTable edges
   (body padding already provides the 24px window inset).  A 3px
   teal stripe on the left visually marks the expanded region. */
tr.fileFunctions > td   { background-color: #fdfdfe; padding: 6px 0;
                          border-left: 3px solid var(--slang-teal); }

/* Embedded function table inside an expanded file row. The colgroup
   widths mirror the parent indexTable so cells line up vertically
   across the file row and its expanded function rows.
   Cells inherit padding / font from their class-specific rules
   above (coverPerHi/Med/Lo, coverNumDflt, coverBar, coverFn …) —
   no generic `table.fnInner td` overrides — which is what makes
   metric cells use the parent's sans-serif and 3px/10px padding,
   while only the function name cell (td.coverFn) stays monospace. */
table.fnInner   { border-collapse: collapse; margin: 0; width: 100%;
                  table-layout: fixed; }
table.fnInner td { overflow-wrap: break-word; }
table.fnInner td.tableHead { font-family: -apple-system, BlinkMacSystemFont,
                             "Segoe UI", Roboto, sans-serif; }
/* fn-specific cols subdivide the parent's File column (47%) into
   Name (38%) + Line (4%) + Calls (5%). The remaining cols reuse
   the parent's classes so the line-coverage / fn / branch columns
   line up vertically with the file-summary row above. */
table.fnInner col.fnNameCol  { width: 38%; }
table.fnInner col.fnLineCol  { width: 4%; }
table.fnInner col.fnCallsCol { width: 5%; }

.sourceUnavailable { color: var(--slang-orange); font-style: italic;
                     padding: 8px; }
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


def _render_page_header(
    title: str,
    breadcrumb_html: str,
    test_name: str,
    test_date: str,
    metrics: List[Metric],
) -> str:
    """Top-of-page chrome: title bar, breadcrumb / test / date,
    and a *transposed* metric grid where columns are
    Lines / Functions / Branches and rows are Coverage / Total / Hit.

    The whole block is wrapped in `<div class="topChrome">` with
    `position: sticky; top: 0` so it stays visible when the user
    scrolls down through the file/source list.
    """
    # The metric grid: one column per metric, three rows for
    # Coverage / Total / Hit. Lines is always present; Functions /
    # Branches only when the LCOV carries them.
    cols_html = "\n".join(
        f'        <td class="metricColHead">{html.escape(m.label)}</td>'
        for m in metrics
    )

    def _rate_cell(m: Metric) -> str:
        if m.total <= 0:
            return '<td class="coverNumDflt">-</td>'
        tier = _tier(m.pct)
        return f'<td class="coverPer{tier}">{m.pct:.1f}&nbsp;%</td>'

    rate_row = "\n".join(
        f"        {_rate_cell(m)}" for m in metrics
    )
    total_row = "\n".join(
        f'        <td class="coverNumDflt">{m.total if m.total else "-"}</td>'
        for m in metrics
    )
    hit_row = "\n".join(
        f'        <td class="coverNumDflt">{m.hit if m.total else "-"}</td>'
        for m in metrics
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
  <table class="metricGrid">
    <thead>
      <tr>
        <td class="metricRowLabel"></td>
{cols_html}
      </tr>
    </thead>
    <tbody>
      <tr>
        <td class="metricRowLabel">Coverage</td>
{rate_row}
      </tr>
      <tr>
        <td class="metricRowLabel">Total</td>
{total_row}
      </tr>
      <tr>
        <td class="metricRowLabel">Hit</td>
{hit_row}
      </tr>
    </tbody>
  </table>
  <table class="chrome" cellpadding="0" cellspacing="0">
    <tr><td class="ruler"></td></tr>
  </table>
</div>
"""


INDEX_TOGGLE_SCRIPT = """\
<script>
(function () {
  // Measure the sticky chrome's height once after layout so the
  // table's <thead> can stick directly underneath.
  function measureChrome() {
    var c = document.querySelector('.topChrome');
    if (c) {
      document.documentElement.style.setProperty(
        '--chrome-h', c.offsetHeight + 'px'
      );
    }
  }
  if (document.readyState === 'complete') measureChrome();
  else window.addEventListener('load', measureChrome);
  window.addEventListener('resize', measureChrome);

  // Per-file toggle: reveals the next sibling fileFunctions row,
  // lazy-cloning the function table from a <template> on first
  // open so the initial DOM stays small.
  function ensureFnLoaded(row) {
    if (row.dataset.loaded === '1') return;
    var tid = row.getAttribute('data-fn-tmpl');
    if (tid) {
      var tmpl = document.getElementById(tid);
      var td = row.querySelector('td');
      if (tmpl && td && td.children.length === 0) {
        td.appendChild(tmpl.content.cloneNode(true));
      }
    }
    row.dataset.loaded = '1';
  }
  function toggleFn(el) {
    var row = el.closest('tr');
    if (!row) return;
    var next = row.nextElementSibling;
    if (!next || !next.classList.contains('fileFunctions')) return;
    var hidden = next.hasAttribute('hidden');
    if (hidden) {
      ensureFnLoaded(next);
      next.removeAttribute('hidden');
      el.textContent = '\\u25BC';
      el.setAttribute('aria-expanded', 'true');
    } else {
      next.setAttribute('hidden', '');
      el.textContent = '\\u25B6';
      el.setAttribute('aria-expanded', 'false');
    }
  }

  // Per-directory toggle: hides every descendant row when collapsing
  // (anything whose data-dir or data-path lives under this dir's
  // path); when expanding, reveals only direct children — files
  // sitting in this dir, plus immediate sub-directory headers — all
  // in collapsed state. The user can drill further by clicking any
  // child chevron.
  function isDescendant(rowKey, parentPath) {
    if (rowKey === null) return false;
    if (parentPath === '') return rowKey !== '';
    return rowKey === parentPath || rowKey.indexOf(parentPath + '/') === 0;
  }
  function pathDepth(p) {
    return p === '' ? 0 : p.split('/').length;
  }
  function toggleDir(el) {
    var path = el.getAttribute('data-path');
    if (path === null) return;
    var collapsing = el.getAttribute('aria-expanded') !== 'false';
    var directChildDepth = pathDepth(path) + 1;

    var rows = document.querySelectorAll(
      'tr.dirHeader, tr.fileSummary, tr.fileFunctions'
    );
    rows.forEach(function (r) {
      // Skip the dir-header row that owns this toggle.
      if (r.classList.contains('dirHeader') &&
          r.getAttribute('data-path') === path) return;

      var rowKey = r.getAttribute('data-dir');
      if (rowKey === null) rowKey = r.getAttribute('data-path');
      if (!isDescendant(rowKey, path)) return;

      if (collapsing) {
        r.setAttribute('hidden', '');
        return;
      }
      // Expanding: reveal only direct children of `path`.
      if (r.classList.contains('fileSummary')) {
        if (rowKey === path) {
          r.removeAttribute('hidden');
          var t = r.querySelector('.fnToggle');
          if (t) {
            t.textContent = '\\u25B6';
            t.setAttribute('aria-expanded', 'false');
          }
        }
      } else if (r.classList.contains('dirHeader')) {
        var rDepth = parseInt(r.getAttribute('data-depth') || '0', 10);
        if (rDepth === directChildDepth) {
          r.removeAttribute('hidden');
          var dt = r.querySelector('.dirToggle');
          if (dt) {
            dt.textContent = '\\u25B6';
            dt.setAttribute('aria-expanded', 'false');
          }
        }
      }
      // fileFunctions rows stay hidden when expanding a directory.
    });

    el.textContent = collapsing ? '\\u25B6' : '\\u25BC';
    el.setAttribute('aria-expanded', collapsing ? 'false' : 'true');
  }

  document.addEventListener('click', function (e) {
    var t = e.target;
    if (!t || !t.classList) return;
    if (t.classList.contains('fnToggle'))   toggleFn(t);
    else if (t.classList.contains('dirToggle')) toggleDir(t);
  });
  document.addEventListener('keydown', function (e) {
    var t = e.target;
    if (!t || !t.classList) return;
    if (e.key !== 'Enter' && e.key !== ' ') return;
    if (t.classList.contains('fnToggle')) {
      e.preventDefault();
      toggleFn(t);
    } else if (t.classList.contains('dirToggle')) {
      e.preventDefault();
      toggleDir(t);
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

    # Group records by their FULL parent directory (relative to the
    # common-prefix-stripped display path). Then walk the directory
    # tree depth-first so each level — `source/`, `source/slang/`,
    # `source/slang/optimizer/`, etc. — gets its own collapsible
    # row independently. Walking lexicographically-sorted dir paths
    # produces the right DFS order naturally.
    sorted_records = sorted(records, key=lambda r: _display_path(r.path, prefix))
    files_by_dir: Dict[str, List[FileRecord]] = collections.OrderedDict()
    for rec in sorted_records:
        display = _display_path(rec.path, prefix)
        if "/" in display:
            dirpath, _, _ = display.rpartition("/")
        else:
            dirpath = ""
        files_by_dir.setdefault(dirpath, []).append(rec)

    # Build the set of all directory paths, including ancestors.
    all_dirs = set()
    for dirpath in files_by_dir:
        parts = dirpath.split("/") if dirpath else []
        for i in range(len(parts) + 1):
            all_dirs.add("/".join(parts[:i]))
    sorted_all_dirs = sorted(all_dirs)

    # Aggregate stats for any directory (its own files + all
    # descendants).
    def _files_under(target: str) -> List[FileRecord]:
        if target == "":
            return [f for fs in files_by_dir.values() for f in fs]
        return [
            f
            for d, fs in files_by_dir.items()
            for f in fs
            if d == target or d.startswith(target + "/")
        ]

    def _depth_of(dirpath: str) -> int:
        return 0 if dirpath == "" else dirpath.count("/") + 1

    def _indent_style(depth: int) -> str:
        # Nested dirs each get an extra ~1.4em of left padding so the
        # tree shape is visible even when row backgrounds are uniform.
        return f"padding-left: calc(10px + {1.4 * depth:.2f}em);"

    rows_html: List[str] = []
    fn_templates: List[Tuple[str, str]] = []  # (template_id, fnInner HTML)

    for dirpath in sorted_all_dirs:
        dir_records = _files_under(dirpath)
        if not dir_records:
            continue
        depth = _depth_of(dirpath)
        last_segment = (dirpath.rsplit("/", 1)[-1] + "/") if dirpath else "(top level)"
        # Aggregated stats for this dir (incl. all descendants).
        d_total_l = sum(r.total_lines for r in dir_records)
        d_hit_l = sum(r.hit_lines for r in dir_records)
        d_rate_l = (100.0 * d_hit_l / d_total_l) if d_total_l else 0.0
        d_total_fn = sum(r.total_functions for r in dir_records)
        d_hit_fn = sum(r.hit_functions for r in dir_records)
        d_rate_fn = (100.0 * d_hit_fn / d_total_fn) if d_total_fn else 0.0
        d_total_br = sum(r.total_branches for r in dir_records)
        d_hit_br = sum(r.hit_branches for r in dir_records)
        d_rate_br = (100.0 * d_hit_br / d_total_br) if d_total_br else 0.0

        dir_path_attr = html.escape(dirpath)
        dir_cells: List[str] = [
            f'              <td class="coverDirectory" '
            f'style="{_indent_style(depth)}">'
            f'<span class="dirToggle" tabindex="0" role="button" '
            f'aria-expanded="true" data-path="{dir_path_attr}" '
            f'aria-label="Toggle directory {html.escape(last_segment)}">▼</span>'
            f"{html.escape(last_segment)}"
            f' <span style="opacity:.7;font-weight:400">'
            f"({len(dir_records)} file{'s' if len(dir_records) != 1 else ''})</span>"
            f"</td>",
            _index_col_group(
                has_bar=True, rate=d_rate_l, total=d_total_l, hit=d_hit_l
            ),
        ]
        if show_fns:
            dir_cells.append(
                _index_col_group(
                    has_bar=False, rate=d_rate_fn, total=d_total_fn, hit=d_hit_fn
                )
            )
        if show_br:
            dir_cells.append(
                _index_col_group(
                    has_bar=False, rate=d_rate_br, total=d_total_br, hit=d_hit_br
                )
            )
        rows_html.append(
            f'            <tr class="dirHeader" data-path="{dir_path_attr}" '
            f'data-depth="{depth}">\n'
            + "\n".join(dir_cells)
            + "\n            </tr>"
        )

        # File rows directly inside this directory (NOT descendants).
        direct = files_by_dir.get(dirpath, [])
        file_depth = depth + 1
        file_indent = _indent_style(file_depth)
        for rec in direct:
            display = _display_path(rec.path, prefix)
            # On nested dirs, show only the file basename in the row;
            # the dir hierarchy is conveyed by the indented dir headers
            # above. The link still points at the full per-file page.
            display_basename = display.rsplit("/", 1)[-1]
            href = file_map[rec.path]
            has_fn_table = bool(rec.functions)
            if has_fn_table:
                toggle = (
                    '<span class="fnToggle" tabindex="0" role="button" '
                    f'aria-label="Toggle functions for {html.escape(display)}" '
                    'aria-expanded="false">▶</span>'
                )
            else:
                toggle = '<span class="fnTogglePlaceholder">▶</span>'
            cells: List[str] = [
                f'              <td class="coverFile" style="{file_indent}">'
                f'{toggle}'
                f'<a href="{html.escape(href)}">{html.escape(display_basename)}</a></td>',
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
                f'            <tr class="fileSummary" data-dir="{dir_path_attr}" '
                f'data-depth="{file_depth}">\n'
                + "\n".join(cells)
                + "\n            </tr>"
            )
            if has_fn_table:
                # Lazy-render: each file's fnInner table is stored in
                # an out-of-table <template>, the row's td is empty
                # until the user clicks the chevron. Keeps initial
                # DOM small even on the 660-file slangc merge.
                tmpl_id = "fn_" + hashlib.sha1(
                    rec.path.encode("utf-8")
                ).hexdigest()[:10]
                fn_templates.append(
                    (tmpl_id, _render_inline_functions_table(
                        rec, href, show_fns=show_fns, show_br=show_br
                    ))
                )
                rows_html.append(
                    f'            <tr class="fileFunctions" data-dir="{dir_path_attr}" '
                    f'data-depth="{file_depth}" data-fn-tmpl="{tmpl_id}" hidden>\n'
                    f'              <td colspan="{cols_for_empty}"></td>\n'
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

    # colgroup widths must match what fnInner uses so the per-function
    # rate / total / hit cells line up vertically with the parent's
    # corresponding "Lines" columns.
    colgroup = '      <colgroup>\n        <col class="colFile">\n'
    colgroup += '        <col class="colLBar">\n'
    colgroup += '        <col class="colLRate">\n'
    colgroup += '        <col class="colLTotal">\n'
    colgroup += '        <col class="colLHit">\n'
    if show_fns:
        colgroup += '        <col class="colFRate">\n'
        colgroup += '        <col class="colFTotal">\n'
        colgroup += '        <col class="colFHit">\n'
    if show_br:
        colgroup += '        <col class="colBRate">\n'
        colgroup += '        <col class="colBTotal">\n'
        colgroup += '        <col class="colBHit">\n'
    colgroup += "      </colgroup>"

    # Out-of-table <template>s for each file's expanded function
    # listing. Browsers parse <template> children as a DocumentFragment
    # without painting them, so initial DOM stays small even when we
    # have thousands of functions across hundreds of files.
    templates_html = "\n".join(
        f'<template id="{tid}">{thtml}</template>'
        for tid, thtml in fn_templates
    )

    body = f"""  <table class="indexTable" cellpadding="1" cellspacing="1" border="0">
{colgroup}
    <thead>
      <tr>
        <td class="tableHead" rowspan="2">File</td>
        <td class="tableHead" colspan="4">Line Coverage</td>{extra_header_cells}
      </tr>
      <tr>
{sub_rate_cells}{extra_sub_rate}
      </tr>
    </thead>
    <tbody>
{chr(10).join(rows_html)}
{prefix_note}    </tbody>
  </table>
{templates_html}
"""

    extra_script = INDEX_TOGGLE_SCRIPT
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


def _render_inline_functions_table(
    record: FileRecord,
    file_href: str,
    show_fns: bool = False,
    show_br: bool = False,
) -> str:
    """Render a Functions table to embed inside an index <td>.

    Each row carries:

      Function | Line | Calls | LineBar | LineRate | LineTotal | LineHit
               | <Function-Coverage cells empty>
               | BrRate | BrTotal | BrHit

    The Bar / Rate / Total / Hit cells line up underneath the parent
    indexTable's "Lines" group; BrRate / BrTotal / BrHit line up
    under the parent's "Branch Coverage" group; the parent's
    "Function Coverage" group is left empty per row (function-level
    coverage isn't a meaningful drilldown when each row IS one
    function).

    "Calls" is the FNDA hit count — the number of times the function
    was invoked during the run. The header carries a tooltip
    explaining that since "Calls" alone can be ambiguous.
    """
    from lcov_io import function_line_coverage, function_branch_coverage

    items = sorted(
        record.functions.items(), key=lambda kv: (kv[1].first_line, kv[0])
    )
    line_cov = function_line_coverage(record)
    br_cov = function_branch_coverage(record)

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

        l_total, l_hit = line_cov.get(name, (0, 0))
        if l_total > 0:
            pct = 100.0 * l_hit / l_total
            tier = _tier(pct)
            rate_cell = f'<td class="coverPer{tier}">{pct:.1f}&nbsp;%</td>'
            bar_cell = (
                f'<td class="coverBar" align="center">{_render_rate_bar(pct)}</td>'
            )
            total_cell = f'<td class="coverNumDflt">{l_total}</td>'
            hit_cell = f'<td class="coverNumDflt">{l_hit}</td>'
        else:
            rate_cell = '<td class="coverNumDflt">-</td>'
            bar_cell = '<td class="coverBar"></td>'
            total_cell = '<td class="coverNumDflt">-</td>'
            hit_cell = '<td class="coverNumDflt">-</td>'

        # Per-function function-coverage cells: 1/1 (covered) when
        # the function was called at least once, 0/1 otherwise.
        # Aligns with the parent's "Function Coverage" group.
        if show_fns:
            fn_hit_int = 1 if fn.hits > 0 else 0
            fn_pct = 100.0 if fn.hits > 0 else 0.0
            fn_tier = _tier(fn_pct)
            fn_rate_cell = (
                f'<td class="coverPer{fn_tier}">{fn_pct:.0f}&nbsp;%</td>'
            )
            fn_total_cell = '<td class="coverNumDflt">1</td>'
            fn_hit_cell = f'<td class="coverNumDflt">{fn_hit_int}</td>'
            fn_cells = fn_rate_cell + fn_total_cell + fn_hit_cell
        else:
            fn_cells = ""

        if show_br:
            b_total, b_hit = br_cov.get(name, (0, 0))
            if b_total > 0:
                bpct = 100.0 * b_hit / b_total
                btier = _tier(bpct)
                br_rate_cell = (
                    f'<td class="coverPer{btier}">{bpct:.1f}&nbsp;%</td>'
                )
                br_total_cell = f'<td class="coverNumDflt">{b_total}</td>'
                br_hit_cell = f'<td class="coverNumDflt">{b_hit}</td>'
            else:
                br_rate_cell = '<td class="coverNumDflt">-</td>'
                br_total_cell = '<td class="coverNumDflt">-</td>'
                br_hit_cell = '<td class="coverNumDflt">-</td>'
            br_cells = br_rate_cell + br_total_cell + br_hit_cell
        else:
            br_cells = ""

        rows.append(
            "        <tr>"
            f'<td class="coverFn">{html.escape(name)}</td>'
            f'<td class="coverNumDflt">{line_cell}</td>'
            f'<td class="{calls_cls}">{fn.hits}</td>'
            f"{bar_cell}{rate_cell}{total_cell}{hit_cell}"
            f"{fn_cells}{br_cells}"
            "</tr>"
        )

    # Matching trailing <col> elements for the parent's Function
    # Coverage and (if present) Branch Coverage groups, so column
    # widths align across the file row and its expanded function rows.
    extra_cols = ""
    if show_fns:
        extra_cols += (
            '          <col class="colFRate">\n'
            '          <col class="colFTotal">\n'
            '          <col class="colFHit">\n'
        )
    if show_br:
        extra_cols += (
            '          <col class="colBRate">\n'
            '          <col class="colBTotal">\n'
            '          <col class="colBHit">\n'
        )

    extra_heads = ""
    if show_fns:
        extra_heads += (
            '          <td class="tableHead" colspan="3">Function Coverage</td>\n'
        )
    if show_br:
        extra_heads += (
            '          <td class="tableHead" colspan="3">Branch Coverage</td>\n'
        )

    calls_tooltip = (
        "Number of invocations of the function during the run "
        "(LCOV FNDA hit count). 0 means uncalled."
    )

    return (
        '<table class="fnInner" cellpadding="1" cellspacing="1" border="0">\n'
        '        <colgroup>\n'
        '          <col class="fnNameCol">\n'
        '          <col class="fnLineCol">\n'
        '          <col class="fnCallsCol">\n'
        '          <col class="colLBar">\n'
        '          <col class="colLRate">\n'
        '          <col class="colLTotal">\n'
        '          <col class="colLHit">\n'
        + extra_cols
        + "        </colgroup>\n"
        "        <tr>\n"
        '          <td class="tableHead">Function</td>\n'
        '          <td class="tableHead">Line</td>\n'
        f'          <td class="tableHead" title="{html.escape(calls_tooltip)}">Calls</td>\n'
        '          <td class="tableHead" colspan="2">Line Coverage</td>\n'
        '          <td class="tableHead">Total</td>\n'
        '          <td class="tableHead">Hit</td>\n'
        + extra_heads
        + "        </tr>\n"
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
