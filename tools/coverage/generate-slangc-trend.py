#!/usr/bin/env python3
"""Generate a self-contained HTML trend page for slangc compiler coverage.

Reads coverage-data.json (all historical records) and supplements with
summary JSONs from reports/history/ for any dates not yet in the export.
Outputs a single self-contained HTML file with an inline SVG line chart.

Usage:
    python3 generate-slangc-trend.py \
        --data reports/coverage-data.json \
        --history reports/history \
        --output reports/slangc-coverage-trend.html
"""
import argparse
import json
import os
import sys


def _git_show(commit, path):
    """Return parsed JSON from a git-show, or None."""
    import subprocess
    try:
        r = subprocess.run(["git", "show", f"{commit}:{path}"],
                           capture_output=True, text=True, check=True)
        return json.loads(r.stdout)
    except Exception:
        return None


def _git_coverage_commits():
    """Return list of (hash, message) for commits with coverage reports."""
    import subprocess
    try:
        r = subprocess.run(
            ["git", "log", "--all", "--grep=multi-platform coverage",
             "--format=%H %s"],
            capture_output=True, text=True, check=True)
        return [line.split(" ", 1) for line in r.stdout.strip().splitlines()
                if line.strip()]
    except Exception:
        return []


def load_records(data_path, history_dir):
    """Load all Linux slangc records from every available source.

    Priority (highest first):
    1. coverage-data.json  — canonical data export, always preferred
    2. Working-tree history/ summary JSONs  — recent runs not yet in export
    3. Git history  — backfills dates culled from the working tree
    """
    import re
    records = {}

    # 1. Canonical export
    if os.path.exists(data_path):
        for r in json.load(open(data_path, encoding="utf-8")):
            if r.get("platform") == "linux" and "slangc_line_coverage" in r:
                records[r["date"]] = r

    # 2. Working-tree history/ entries
    if os.path.isdir(history_dir):
        for entry in sorted(os.listdir(history_dir)):
            combined = os.path.join(history_dir, entry, "combined-summary.json")
            if not os.path.exists(combined):
                continue
            try:
                d = json.load(open(combined, encoding="utf-8"))
            except (json.JSONDecodeError, OSError):
                continue
            date = d.get("date") or entry[:10]
            linux = d.get("platforms", {}).get("linux", {})
            if date not in records and "slangc_line_coverage" in linux:
                records[date] = {**linux, "date": date,
                                  "commit": d.get("commit", ""),
                                  "platform": "linux"}

    # 3. Git history — backfill dates that have been deleted from working tree
    for parts in _git_coverage_commits():
        if len(parts) != 2:
            continue
        commit_hash, message = parts
        m = re.search(r"(\d{4}-\d{2}-\d{2})", message)
        date = m.group(1) if m else None
        sc = re.search(r"\(([0-9a-f]{7,40})\)", message)
        slang_commit = sc.group(1) if sc else None
        if not date or date in records:
            continue
        # Try the combined-summary path first, fall back to linux summary
        paths = []
        if slang_commit:
            paths.append(f"reports/history/{date}-{slang_commit}/combined-summary.json")
        paths.append("reports/latest/linux/coverage-summary.json")
        for path in paths:
            d = _git_show(commit_hash, path)
            if d is None:
                continue
            # combined-summary has platforms.linux; coverage-summary is flat
            linux = d.get("platforms", {}).get("linux", d)
            if "slangc_line_coverage" in linux and pct(linux["slangc_line_coverage"]) is not None:
                records[date] = {**linux, "date": date,
                                  "commit": slang_commit or commit_hash[:9],
                                  "platform": "linux"}
                break

    return sorted(records.values(), key=lambda r: r["date"])


def pct(val, vmin=50.0):
    """Parse percentage string/number; return None for missing or implausibly low values."""
    if val is None:
        return None
    try:
        v = float(str(val).rstrip("%"))
        return v if v >= vmin else None
    except ValueError:
        return None


def render(records):
    if not records:
        return "<p>No data.</p>"

    dates = [r["date"] for r in records]
    n = len(dates)

    series = {
        "Line":     ("slangc_line_coverage",     "#2563eb", ""),
        "Function": ("slangc_function_coverage",  "#16a34a", "6 3"),
        "Branch":   ("slangc_branch_coverage",    "#dc2626", "3 3"),
        "Region":   ("slangc_region_coverage",    "#9333ea", "8 4"),
    }

    W, H = 900, 320
    ml, mt, mr, mb = 60, 30, 20, 60
    pw, ph = W - ml - mr, H - mt - mb
    ymin, ymax = 60.0, 100.0

    def y_px(v):
        return mt + ph - (v - ymin) / (ymax - ymin) * ph

    def x_px(i):
        return ml + i / max(n - 1, 1) * pw

    # Gridlines
    grid_lines = []
    for v in range(60, 101, 5):
        y = y_px(v)
        grid_lines.append(
            f'<line x1="{ml}" y1="{y:.1f}" x2="{ml+pw}" y2="{y:.1f}" '
            f'stroke="#eee" stroke-width="1"/>'
            f'<text x="{ml-6}" y="{y+4:.1f}" text-anchor="end" '
            f'fill="#888" font-size="11">{v}</text>')

    # Series polylines + latest dot + label
    # Build separate segments for contiguous runs to avoid connecting across gaps.
    polylines = []
    for label, (key, color, dash) in series.items():
        vals = [pct(r.get(key)) for r in records]
        da = f' stroke-dasharray="{dash}"' if dash else ""
        # Split into contiguous segments
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
            polylines.append(
                f'<polyline points="{pts}" stroke="{color}" stroke-width="2" '
                f'fill="none"{da}><title>{label} coverage</title></polyline>')
        # Dot + value label on latest point
        li = next((i for i in reversed(range(n)) if vals[i] is not None), None)
        if li is not None:
            x, y = x_px(li), y_px(vals[li])
            polylines.append(
                f'<circle cx="{x:.1f}" cy="{y:.1f}" r="4" fill="{color}"/>'
                f'<text x="{x+6:.1f}" y="{y+4:.1f}" fill="{color}" '
                f'font-size="11" font-weight="600">{vals[li]:.1f}%</text>')

    # X-axis date labels (rotated, evenly spaced)
    stride = max(1, n // 8)
    x_labels = []
    for i in range(0, n, stride):
        x = x_px(i)
        x_labels.append(
            f'<text x="{x:.1f}" y="{mt+ph+18}" text-anchor="end" '
            f'fill="#666" font-size="10" '
            f'transform="rotate(-45 {x:.1f} {mt+ph+18})">{dates[i]}</text>')

    svg = "\n  ".join([
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" '
        f'viewBox="0 0 {W} {H}" font-family="sans-serif">',
        f'<rect width="{W}" height="{H}" fill="white"/>',
        "\n  ".join(grid_lines),
        f'<line x1="{ml}" y1="{mt}" x2="{ml}" y2="{mt+ph}" stroke="#ccc"/>',
        f'<line x1="{ml}" y1="{mt+ph}" x2="{ml+pw}" y2="{mt+ph}" stroke="#ccc"/>',
        "\n  ".join(polylines),
        "\n  ".join(x_labels),
        f'<text x="{ml-40}" y="{mt+ph//2}" text-anchor="middle" fill="#555" '
        f'font-size="11" transform="rotate(-90 {ml-40} {mt+ph//2})">Coverage %</text>',
        "</svg>",
    ])

    # Legend
    legend_items = "".join(
        f'<span style="display:inline-flex;align-items:center;margin-right:16px">'
        f'<svg width="24" height="8"><line x1="0" y1="4" x2="24" y2="4" '
        f'stroke="{color}" stroke-width="2" stroke-dasharray="{dash}"/></svg>'
        f'&nbsp;{label}</span>'
        for label, (_, color, dash) in series.items())

    # Summary table
    latest = records[-1]
    prev   = records[-2] if n > 1 else None

    def delta_str(key):
        cur = pct(latest.get(key))
        if prev is None or cur is None:
            return ""
        pre = pct(prev.get(key))
        if pre is None:
            return ""
        d = cur - pre
        col = "#16a34a" if d >= 0 else "#dc2626"
        sign = "+" if d >= 0 else ""
        return f'<span style="color:{col};font-size:12px"> ({sign}{d:.2f}pp)</span>'

    td = 'style="padding:8px 12px;border:1px solid #e5e7eb"'
    table_rows = []
    for label, (key, _, _) in series.items():
        val = pct(latest.get(key))
        val_str = f"{val:.2f}%" if val is not None else "—"
        detail = ""
        if key == "slangc_line_coverage":
            h = latest.get("slangc_lines_hit")
            f = latest.get("slangc_lines_found")
            if h and f:
                detail = (f'<br><span style="color:#888;font-size:11px">'
                          f'{int(h):,} / {int(f):,} lines</span>')
        table_rows.append(
            f'<tr><td {td}>{label}</td>'
            f'<td {td}><b>{val_str}</b>{delta_str(key)}{detail}</td></tr>')

    th = ('style="padding:8px 12px;text-align:left;border:1px solid #e5e7eb;'
          'background:#f3f4f6"')
    table = (
        f'<table style="border-collapse:collapse;font-size:14px;width:420px">'
        f'<thead><tr><th {th}>Metric</th><th {th}>Latest</th></tr></thead>'
        f'<tbody>{"".join(table_rows)}</tbody></table>')

    return f"""
<div style="margin-bottom:24px">
  <h2 style="font-size:18px;margin:0 0 4px">slangc compiler coverage trend — Linux x86-64</h2>
  <p style="color:#555;font-size:13px;margin:0 0 12px">
    Latest: <b>{latest["date"]}</b>
    ({latest.get("commit", "?")})
    &nbsp;·&nbsp; {n} data points
  </p>
  <div style="overflow:auto">{svg}</div>
  <div style="margin-top:8px;font-size:12px;color:#555">{legend_items}</div>
</div>
{table}
"""


def main():
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--data",    default="reports/coverage-data.json")
    ap.add_argument("--history", default="reports/history")
    ap.add_argument("--output",  default="reports/slangc-coverage-trend.html")
    args = ap.parse_args()

    records = load_records(args.data, args.history)
    if not records:
        sys.exit("No slangc Linux coverage records found.")

    body = render(records)
    html = f"""<!doctype html>
<meta charset="utf-8">
<title>slangc Coverage Trend — Slang</title>
<style>
body {{
  font: 14px/1.6 -apple-system, Segoe UI, Roboto, sans-serif;
  margin: 0; color: #1a1a1a; background: #fafafa;
}}
.wrap {{max-width: 1000px; margin: 0 auto; padding: 28px}}
h1 {{font-size: 22px; margin: 0 0 4px}}
.sub {{color: #666; margin: 0 0 24px; font-size: 13px}}
</style>
<div class="wrap">
  <h1>Slang — slangc compiler code coverage</h1>
  <p class="sub">
    Daily tip-of-tree coverage of the slangc compiler pipeline, Linux x86-64.
    <a href="index.html" style="margin-left:16px">← All coverage reports</a>
  </p>
  {body}
</div>
"""

    os.makedirs(os.path.dirname(os.path.abspath(args.output)), exist_ok=True)
    with open(args.output, "w", encoding="utf-8") as fh:
        fh.write(html)
    print(f"wrote {args.output}  ({n} records)" if (n := len(records)) else
          f"wrote {args.output}")


if __name__ == "__main__":
    main()
