"""Shared HTML/SVG rendering primitives for the report generators.

report.py, breakdown.py, and sweep_report.py each grew rendering code
independently; the pieces every generator needs identically live here.
The stacked-area machinery stays in breakdown.py and the sweep panels in
sweep_report.py — they are genuinely different charts — but the escaper
and the simple multi-series line panel (used by the memory pages, where
components do NOT tile a total and a stacked area would lie) are shared.
"""


def esc(s):
    """HTML-escape &, <, > — sufficient because interpolated values are
    controlled workload/tag/date names, never user input (no quote escaping
    needed for the attribute contexts the generators emit)."""
    return str(s).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")


def line_panel(labels, series, title, unit="", width=620, height=300):
    """One SVG panel: multiple line series over a shared categorical x axis.

    labels — x-axis category names (release tags / daily dates), evenly
    spaced; every ~len/6th label is drawn to avoid clutter.
    series — [(name, css_color, [value_or_None per label])]; None gaps split
    the polyline rather than interpolating through missing points.
    unit   — y-axis legend suffix (e.g. "MiB").

    Y always starts at 0 so panel-to-panel comparisons stay honest.
    """
    pad_l, pad_r, pad_t, pad_b = 56, 10, 26, 34
    pw, ph = width - pad_l - pad_r, height - pad_t - pad_b
    vals = [v for _n, _c, vs in series for v in vs if v is not None]
    # The y domain always includes 0 (honest panel-to-panel comparison) and
    # extends below it when values are negative (e.g. an RSS delta can be).
    top = max([v for v in vals] + [0.0]) * 1.08 if vals else 1.0
    top = top or 1.0
    bot = min([v for v in vals] + [0.0])
    bot = bot * 1.08 if bot < 0 else 0.0
    n = max(len(labels), 2)

    def x(i):
        return pad_l + pw * i / (n - 1)

    def y(v):
        return pad_t + ph * (1 - (v - bot) / (top - bot))

    s = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" '
         f'font-family="system-ui,sans-serif" font-size="10">']
    s.append(f'<text x="{pad_l}" y="15" font-size="12" font-weight="600" '
             f'fill="#333">{esc(title)}</text>')
    for frac in (0.0, 0.5, 1.0):
        gv = bot + (top - bot) * frac
        gy = y(gv)
        s.append(f'<line x1="{pad_l}" y1="{gy:.1f}" x2="{width - pad_r}" y2="{gy:.1f}" '
                 f'stroke="#eee"/>')
        s.append(f'<text x="{pad_l - 4}" y="{gy + 3:.1f}" text-anchor="end" '
                 f'fill="#666">{gv:.0f}{esc(unit)}</text>')
    step = max(1, n // 6)
    for i, lab in enumerate(labels):
        if i % step == 0 or i == n - 1:
            s.append(f'<text x="{x(i):.1f}" y="{height - 8}" text-anchor="middle" '
                     f'fill="#666">{esc(str(lab))}</text>')
    lx = pad_l
    for name, color, vs in series:
        run = []

        def flush(run):
            # A lone point (a gap on both sides, or a single-measurement
            # series — the first release with memory data) gets a visible
            # marker; a polyline needs two points and would draw nothing.
            if len(run) == 1:
                cx, cy = run[0].split(",")
                s.append(f'<circle cx="{cx}" cy="{cy}" r="3" fill="{color}"/>')
            elif len(run) > 1:
                s.append(f'<polyline points="{" ".join(run)}" fill="none" '
                         f'stroke="{color}" stroke-width="2"/>')

        for i, v in enumerate(vs):
            if v is None:
                flush(run)
                run = []
                continue
            run.append(f"{x(i):.1f},{y(v):.1f}")
        flush(run)
        s.append(f'<rect x="{lx}" y="{pad_t - 8}" width="9" height="9" fill="{color}"/>')
        s.append(f'<text x="{lx + 12}" y="{pad_t}" fill="#444">{esc(name)}</text>')
        lx += 14 + 7 * len(name)
    s.append("</svg>")
    return "".join(s)
