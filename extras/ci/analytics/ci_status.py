#!/usr/bin/env python3
"""
CI Status Page Generator

Reads hand-edited status_updates.json from the analytics output directory
and generates status.html for the CI analytics dashboard.

The status_updates.json file lives in the analytics repo (shader-slang/slang-ci-analytics)
so it can be updated without PRing the main slang repo.

Usage:
    python3 ci_status.py --output ./ci_analytics_repo
"""

import argparse
import html as html_mod
import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from ci_visualization import page_template

SEVERITY_COLORS = {
    "info": ("#0d6efd", "#cfe2ff"),
    "warning": ("#fd7e14", "#fff3cd"),
    "critical": ("#dc3545", "#f8d7da"),
}


def parse_args():
    parser = argparse.ArgumentParser(
        description="Generate CI status page from status_updates.json."
    )
    parser.add_argument(
        "--output", default="ci_analytics", help="Output directory (also where status_updates.json is read from)"
    )
    return parser.parse_args()


def load_status_updates(output_dir):
    """Load status entries from the analytics repo directory."""
    path = os.path.join(output_dir, "status_updates.json")
    if not os.path.exists(path):
        return []
    try:
        with open(path, encoding="utf-8") as f:
            data = json.load(f)
        entries = data.get("entries", [])
        if not isinstance(entries, list):
            print(f"Warning: status_updates.json 'entries' is not a list", file=sys.stderr)
            return []
        return entries
    except (OSError, json.JSONDecodeError) as e:
        print(f"Warning: could not read status_updates.json: {e}", file=sys.stderr)
        return []


def render_entry(entry):
    """Render a single status entry as an HTML card."""
    sev = entry.get("severity", "info")
    fg, bg = SEVERITY_COLORS.get(sev, SEVERITY_COLORS["info"])
    title = html_mod.escape(str(entry.get("title", "")))
    body = html_mod.escape(str(entry.get("body", ""))).replace("\n", "<br>")
    date = html_mod.escape(str(entry.get("date", "")))
    author = html_mod.escape(str(entry.get("author", "")))

    author_html = f" &mdash; {author}" if author else ""
    return f"""<div style="border-left:4px solid {fg};background:{bg};padding:15px 20px;margin-bottom:15px;border-radius:4px">
  <span style="background:{fg};color:white;padding:2px 8px;border-radius:3px;font-size:0.8em;text-transform:uppercase">{html_mod.escape(sev)}</span>
  <strong style="margin-left:10px;font-size:1.1em">{title}</strong>
  <div style="color:#6c757d;font-size:0.85em;margin-top:4px">{date}{author_html}</div>
  <div style="margin-top:8px">{body}</div>
</div>"""


def generate_status_html(output_dir):
    """Generate status.html from status_updates.json."""
    entries = load_status_updates(output_dir)

    # Filter to visible entries only (default visible if not specified)
    visible = [e for e in entries if e.get("visible", True)]

    # Sort by date descending
    visible.sort(key=lambda e: e.get("date", ""), reverse=True)

    if visible:
        cards = "\n".join(render_entry(e) for e in visible)
        body = f"""<h1>CI Status</h1>
<p style="color:#6c757d">Known issues and maintenance notices. Edit <code>status_updates.json</code> in the
<a href="https://github.com/shader-slang/slang-ci-analytics">analytics repo</a> to update.</p>
{cards}"""
    else:
        body = """<h1>CI Status</h1>
<p style="color:#6c757d">Known issues and maintenance notices. Edit <code>status_updates.json</code> in the
<a href="https://github.com/shader-slang/slang-ci-analytics">analytics repo</a> to update.</p>
<div style="background:#d1e7dd;border-left:4px solid #198754;padding:15px 20px;border-radius:4px;color:#0f5132">
  <strong>All Systems Operational</strong> &mdash; No known issues.
</div>"""

    os.makedirs(output_dir, exist_ok=True)
    with open(os.path.join(output_dir, "status.html"), "w") as f:
        f.write(page_template("Status", body, "Status"))


def main():
    args = parse_args()
    generate_status_html(args.output)
    print(f"Generated status.html in {args.output}")


if __name__ == "__main__":
    main()
