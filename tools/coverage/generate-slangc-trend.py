#!/usr/bin/env python3
"""Generate a self-contained HTML trend page for slangc compiler coverage.

Reads reports/slangc-coverage-history.json and writes a self-contained HTML
file with inline SVG line charts — one panel for slangc line coverage across
all platforms, plus per-platform detail panels.

Usage:
    python3 generate-slangc-trend.py \
        --history reports/slangc-coverage-history.json \
        --output  reports/slangc-coverage-trend.html
"""
import argparse
import json
import os
import sys


def pct(val):
    if val is None:
        return None
    try:
        return float(str(val).rstrip("%"))
    except ValueError:
        return None


def avg(*vals):
    """Mean of non-None values, or None if all None."""
    vs = [v for v in vals if v is not None]
    return sum(vs) / len(vs) if vs else None


def build_svg(dates, series_list, ymin=60.0, ymax=100.0,
              W=860, H=280, title=""):
    """Return an SVG string. series_list = [(label, color, dash, vals), ...]"""
    ml, mt, mr, mb = 60, title and 36 or 10, 20, 55
    pw, ph = W - ml - mr, H - mt - mb
    n = len(dates)

    def y_px(v):
        return mt + ph - (v - ymin) / (ymax - ymin) * ph

    def x_px(i):
        return ml + i / max(n - 1, 1) * pw

    parts = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" '
        f'viewBox="0 0 {W} {H}" font-family="sans-serif">',
        f'<rect width="{W}" height="{H}" fill="white"/>',
    ]
    if title:
        parts.append(f'<text x="{ml}" y="22" font-size="13" font-weight="600" '
                     f'fill="#333">{title}</text>')

    # Gridlines
    for v in range(int(ymin), int(ymax) + 1, 5):
        y = y_px(v)
        parts.append(f'<line x1="{ml}" y1="{y:.1f}" x2="{ml+pw}" y2="{y:.1f}" '
                     f'stroke="#eee" stroke-width="1"/>'
                     f'<text x="{ml-5}" y="{y+4:.1f}" text-anchor="end" '
                     f'fill="#999" font-size="10">{v}</text>')
    parts.append(f'<line x1="{ml}" y1="{mt}" x2="{ml}" y2="{mt+ph}" stroke="#ccc"/>')
    parts.append(f'<line x1="{ml}" y1="{mt+ph}" x2="{ml+pw}" y2="{mt+ph}" stroke="#ccc"/>')

    # Series
    for label, color, dash, vals in series_list:
        da = f' stroke-dasharray="{dash}"' if dash else ""
        seg, segs = [], []
        for i, v in enumerate(vals):
            if v is not None:
                seg.append((i, v))
            else:
                if len(seg) >= 2:
                    segs.append(seg)
                seg = []
        if len(seg) >= 2:
            segs.append(seg)
        for seg in segs:
            pts = " ".join(f"{x_px(i):.1f},{y_px(v):.1f}" for i, v in seg)
            parts.append(f'<polyline points="{pts}" stroke="{color}" '
                         f'stroke-width="2" fill="none"{da}>'
                         f'<title>{label}</title></polyline>')
        li = next((i for i in reversed(range(n)) if vals[i] is not None), None)
        if li is not None:
            x, y = x_px(li), y_px(vals[li])
            parts.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="3.5" fill="{color}"/>'
                         f'<text x="{x+5:.1f}" y="{y+4:.1f}" fill="{color}" '
                         f'font-size="10" font-weight="600">{vals[li]:.1f}%</text>')

    # X-axis labels
    stride = max(1, n // 8)
    for i in range(0, n, stride):
        x = x_px(i)
        parts.append(f'<text x="{x:.1f}" y="{mt+ph+16}" text-anchor="end" '
                     f'fill="#666" font-size="9" '
                     f'transform="rotate(-45 {x:.1f} {mt+ph+16})">{dates[i]}</text>')

    parts.append(f'<text x="{ml-40}" y="{mt+ph//2}" text-anchor="middle" '
                 f'fill="#777" font-size="10" '
                 f'transform="rotate(-90 {ml-40} {mt+ph//2})">Coverage %</text>')
    parts.append("</svg>")
    return "\n".join(parts)


def legend_html(items):
    return "".join(
        f'<span style="display:inline-flex;align-items:center;margin-right:14px">'
        f'<svg width="22" height="8"><line x1="0" y1="4" x2="22" y2="4" '
        f'stroke="{c}" stroke-width="2" stroke-dasharray="{d}"/></svg>'
        f'&nbsp;<span style="font-size:12px;color:#555">{lbl}</span></span>'
        for lbl, c, d in items)


def render(records):
    if not records:
        return "<p>No data.</p>"

    dates = [r["date"] for r in records]
    n = len(dates)
    latest = records[-1]
    prev   = records[-2] if n > 1 else None

    # ── Panel 1: slangc line coverage, all platforms + combined ──────────────
    linux_line   = [pct(r.get("linux_slangc_line_coverage"))   for r in records]
    macos_line   = [pct(r.get("macos_slangc_line_coverage"))   for r in records]
    windows_line = [pct(r.get("windows_slangc_line_coverage")) for r in records]
    combined     = [avg(linux_line[i], macos_line[i], windows_line[i])
                    for i in range(n)]

    panel1_series = [
        ("Linux",    "#2563eb", "",    linux_line),
        ("macOS",    "#16a34a", "6 3", macos_line),
        ("Windows",  "#dc2626", "3 3", windows_line),
        ("Combined", "#7c3aed", "1 4", combined),
    ]
    svg1 = build_svg(dates, panel1_series, title="slangc Line Coverage — all platforms")
    leg1 = legend_html([(lbl, c, d) for lbl, c, d, _ in panel1_series])

    # ── Panel 2: Linux detail (line / function / branch / region) ─────────────
    linux_series = [
        ("Line",     "#2563eb", "",    [pct(r.get("linux_slangc_line_coverage"))     for r in records]),
        ("Function", "#16a34a", "6 3", [pct(r.get("linux_slangc_function_coverage")) for r in records]),
        ("Branch",   "#dc2626", "3 3", [pct(r.get("linux_slangc_branch_coverage"))   for r in records]),
        ("Region",   "#9333ea", "8 4", [pct(r.get("linux_slangc_region_coverage"))   for r in records]),
    ]
    svg2 = build_svg(dates, linux_series, title="Linux — all metrics")
    leg2 = legend_html([(lbl, c, d) for lbl, c, d, _ in linux_series])

    # ── Panel 3: macOS detail ─────────────────────────────────────────────────
    macos_series = [
        ("Line",     "#2563eb", "",    [pct(r.get("macos_slangc_line_coverage"))     for r in records]),
        ("Function", "#16a34a", "6 3", [pct(r.get("macos_slangc_function_coverage")) for r in records]),
        ("Branch",   "#dc2626", "3 3", [pct(r.get("macos_slangc_branch_coverage"))   for r in records]),
        ("Region",   "#9333ea", "8 4", [pct(r.get("macos_slangc_region_coverage"))   for r in records]),
    ]
    svg3 = build_svg(dates, macos_series, title="macOS — all metrics")
    leg3 = legend_html([(lbl, c, d) for lbl, c, d, _ in macos_series])

    # ── Summary table ──────────────────────────────────────────────────────────
    def delta(key):
        if prev is None: return ""
        cur = pct(latest.get(key))
        pre = pct(prev.get(key))
        if cur is None or pre is None: return ""
        d = cur - pre
        col = "#16a34a" if d >= 0 else "#dc2626"
        return f'<span style="color:{col};font-size:11px"> ({"+" if d>=0 else ""}{d:.2f}pp)</span>'

    td = 'style="padding:6px 12px;border:1px solid #e5e7eb;font-size:13px"'
    th = 'style="padding:6px 12px;border:1px solid #e5e7eb;background:#f3f4f6;font-size:13px;text-align:left"'

    def row(metric, linux_k, macos_k, win_k):
        lv = pct(latest.get(linux_k))
        mv = pct(latest.get(macos_k))
        wv = pct(latest.get(win_k))
        fmt = lambda v, k: (f"<b>{v:.2f}%</b>{delta(k)}" if v is not None else "—")
        return (f'<tr><td {td}>{metric}</td>'
                f'<td {td}>{fmt(lv, linux_k)}</td>'
                f'<td {td}>{fmt(mv, macos_k)}</td>'
                f'<td {td}>{fmt(wv, win_k) if wv is not None else "—"}</td></tr>')

    table = f"""<table style="border-collapse:collapse;width:100%;max-width:640px;margin-top:16px">
<thead><tr>
  <th {th}>Metric</th><th {th}>Linux</th><th {th}>macOS</th><th {th}>Windows</th>
</tr></thead><tbody>
{row("Line",     "linux_slangc_line_coverage",     "macos_slangc_line_coverage",     "windows_slangc_line_coverage")}
{row("Function", "linux_slangc_function_coverage", "macos_slangc_function_coverage", None)}
{row("Branch",   "linux_slangc_branch_coverage",   "macos_slangc_branch_coverage",   None)}
{row("Region",   "linux_slangc_region_coverage",   "macos_slangc_region_coverage",   None)}
</tbody></table>""".replace(
    f'<td {td}>—</td>', f'<td {td} style="color:#aaa">—</td>')

    card = 'style="background:#fff;border:1px solid #eee;border-radius:6px;padding:12px;margin-bottom:20px;overflow:auto"'

    return f"""
<p style="color:#555;font-size:13px;margin:0 0 16px">
  Latest: <b>{latest["date"]}</b> ({latest.get("commit","?")})
  &nbsp;·&nbsp; {n} data points
</p>

<div {card}>
  {svg1}
  <div style="margin-top:6px">{leg1}</div>
</div>

<div style="display:grid;grid-template-columns:1fr 1fr;gap:16px">
  <div {card}>
    {svg2}
    <div style="margin-top:6px">{leg2}</div>
  </div>
  <div {card}>
    {svg3}
    <div style="margin-top:6px">{leg3}</div>
  </div>
</div>

<h3 style="font-size:15px;margin:8px 0 4px">Latest values</h3>
{table}
"""


def main():
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--history", default="reports/slangc-coverage-history.json")
    ap.add_argument("--output",  default="reports/slangc-coverage-trend.html")
    args = ap.parse_args()

    if not os.path.exists(args.history):
        print(f"history file not found: {args.history}; skipping trend generation")
        return

    records = json.load(open(args.history, encoding="utf-8"))
    if not records:
        print("No records in history file; skipping trend generation")
        return

    body = render(records)
    html = f"""<!doctype html>
<meta charset="utf-8">
<title>slangc Coverage Trend — Slang</title>
<style>
body {{font:14px/1.6 -apple-system,Segoe UI,Roboto,sans-serif;
      margin:0;color:#1a1a1a;background:#fafafa}}
.wrap {{max-width:1100px;margin:0 auto;padding:28px}}
h1 {{font-size:22px;margin:0 0 4px}}
.sub {{color:#666;margin:0 0 20px;font-size:13px}}
</style>
<div class="wrap">
  <h1>Slang — slangc compiler code coverage</h1>
  <p class="sub">
    Daily tip-of-tree slangc compiler coverage.
    <a href="index.html" style="margin-left:16px">← All coverage reports</a>
  </p>
  {body}
</div>
"""

    os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
    with open(args.output, "w", encoding="utf-8") as fh:
        fh.write(html)
    print(f"wrote {args.output}  ({len(records)} records)")


if __name__ == "__main__":
    main()
