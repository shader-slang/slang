"""Per-subtest daily/release charts with explicit measurement compatibility groups."""
import hashlib
import html
import json
import math
import re

from lib import render
import perf

COLORS = ("#1769aa", "#e07818", "#8055ad", "#16816a")


def esc(value):
    return html.escape(str(value), quote=True)


def filename(case):
    return "test-" + re.sub(r"[^a-zA-Z0-9_-]", "-", case) + ".html"


def fingerprint(value):
    return hashlib.sha256(json.dumps(value, sort_keys=True).encode()).hexdigest()[:12]


def duration(value):
    return value if isinstance(value, (int, float)) and math.isfinite(value) and value >= 0 else None


def runtime_contract(event):
    bundle, case = event["bundle"], event["case"]
    return dict(envelope={key: bundle.get(key) for key in (
        "schema_version", "workload_version", "environment", "generator_sha256", "runner_source_sha256")},
        policy=perf.policy(bundle["config"]),
        workload={key: case.get(key) for key in (
            "size", "source_sha256", "elements", "width", "batch", "measurement_mode", "adapter",
            "backend", "rhi_revision", "timestamp_frequency", "scene")})


def reference_contract(event):
    data, row = event["references"], event["reference"]
    return dict(envelope={key: data.get(key) for key in (
        "schema_version", "workload_version", "environment", "measurement", "runner_sources")},
        workload={key: row.get(key) for key in (
            "backend", "case", "batch", "adapter", "rhi_revision", "slang_source_sha256", "reference_source_sha256")},
        toolchain={key: row.get("reference_compilation", {}).get(key) for key in (
            "tool_sha256", "source_sha256", "binary_sha256")})


def groups(events, reference=False):
    """A changed contract creates another graph, never a connected performance trend."""
    result = {}
    for event in events:
        if reference and not event.get("reference"):
            continue
        case = event["case"]
        if not reference and "elements" not in case:
            continue
        try:
            contract = reference_contract(event) if reference else runtime_contract(event)
        except (KeyError, TypeError, ValueError):
            continue
        key = fingerprint(contract)
        result.setdefault(key, {"contract": contract, "points": {}})["points"][event["id"]] = event
    return result


def valid_case(event):
    case = event["case"]
    return case.get("status") == "ok" and case.get("validated")


def graph(events, series, title, unit):
    labels = [e["record"]["label"] if e["kind"] == "release" else
              e["record"]["label"][:10] + (f" · {i + 1}" if len(events) > 1 else "")
              for i, e in enumerate(events)]
    titles = [e["record"]["label"] + " / " + e["record"]["attempt"] for e in events]
    if not any(v is not None for _, _, values in series for v in values):
        return f'<p class="note">{esc(title)}: no valid measurements.</p>'
    return '<div class="scroll chart">' + render.line_panel(
        labels, series, title, unit=unit, width=1000, height=330, point_titles=titles,
        tick_format=".3g") + '</div>'


def cohort(events, group, reference=False):
    points = group["points"]
    first = next(iter(points.values()))
    case = first["reference"] if reference else first["case"]
    elements = first["references"]["measurement"]["elements"] if reference else case["elements"]
    environment = first["references"]["environment"] if reference else first["bundle"]["environment"]
    body = (f'<p class="note">{esc(case.get("adapter", ""))} · driver {esc(environment.get("driver_id", ""))} · '
            f'{elements:,} elements · batch {case["batch"]} · contract {fingerprint(group["contract"])}.</p>')
    series = []
    for index, implementation in enumerate(("slang", "reference") if reference else ("slang",)):
        values = []
        for event in events:
            point = points.get(event["id"])
            value = None
            if point:
                if reference:
                    summaries = point["reference"]["summary"]
                    if point["references"].get("complete") and all(summaries[k]["valid"] for k in ("slang", "reference")):
                        value = duration(summaries[implementation].get("median_ms"))
                elif valid_case(point):
                    value = duration(point["case"].get("timers", {}).get("gpu_iteration_ms", {}).get("median"))
            values.append(value * 1000 if value is not None else None)
        series.append(("Slang" if implementation == "slang" else "Equivalent reference", COLORS[index], values))
    body += graph(events, series, "Matched Slang and reference runtime" if reference else "Full-suite runtime", "µs")
    if not reference:
        entries = sorted({r["workload"] for point in points.values() for r in point["case"].get("compile_runs", [])})
        for timer, title in (("wall_ms", "Paired CLI compile wall time"), ("compileInner", "Compiler internal total")):
            compile_series = []
            for i, name in enumerate(entries):
                values = []
                for event in events:
                    point = points.get(event["id"])
                    run = next((r for r in point["case"].get("compile_runs", []) if r["workload"] == name), None) if point else None
                    stats = (run.get("wall_ms") if timer == "wall_ms" else run.get("timers", {}).get(timer)) if run else None
                    values.append(duration(stats.get("median")) if stats and run.get("ok") and run.get("got_timers") else None)
                short = name.removeprefix("runtime_" + first["case"]["workload"]).lstrip("_") or "main"
                compile_series.append((short, COLORS[i % len(COLORS)], values))
            body += graph(events, compile_series, title, "ms")
    body += '<details><summary>Measurement contract</summary><pre>' + esc(json.dumps(group["contract"], indent=2)) + '</pre></details>'
    return body


def ordered(events, kind):
    # A re-run replaces its earlier attempt on the chart, including a failed re-run.
    # Every attempt remains available in the run archive.
    by_label = {}
    for event in sorted(events, key=lambda e: e["record"]["created_unix"]):
        if event["kind"] == kind:
            by_label[event["record"]["label"]] = event
    if kind == "release":
        labels = sorted((label for label in by_label if re.fullmatch(r"v\d{4}\.\d+", label)),
                        key=lambda label: tuple(map(int, label[1:].split("."))))[-5:]
        return [by_label[label] for label in labels]
    return sorted(by_label.values(), key=lambda e: (e["record"]["label"][:10],
                                                     e["record"]["created_unix"]))[-30:]


def write_subtests(output, events, page):
    tests = {}
    for event in events:
        tests.setdefault(event["case"]["workload"], {}).setdefault(event["backend"], []).append(event)
    rows = []
    for name in list(perf.workloads.CASES) + sorted(set(tests) - set(perf.workloads.CASES)):
        backends = tests.get(name, {})
        links = [f'<a href="{filename(name)}#{esc(backend)}">{esc(backend)}</a>' for backend in backends]
        rows.append(f'<tr><td><a href="{filename(name)}">{esc(name)}</a></td>'
                    f'<td>{esc(perf.workloads.CASES.get(name, "archived"))}</td><td>{" · ".join(links) or "No measurements"}</td></tr>')
        body = ('<p>Runtime and paired compilation for this subtest. Daily and release histories are separate. '
                'Hover over points for exact values; failed or missing measurements leave gaps. '
                'Changes to GPU, driver, workload size or measurement contract start separate graphs.</p>'
                '<p>Reference curves use Slang and equivalent implementations measured at matching dimensions. '
                'Full-suite timings may use larger dimensions. Compile graphs measure repeated Slang CLI compilation '
                'of the executed source; reference compilation is a one-time observation and is not overlaid. '
                'Image stages remain separate, not summed medians.</p>')
        if not backends:
            body += '<p>No measurements have been archived for this subtest.</p>'
        body += '<p>' + ' · '.join(links) + '</p>'
        for backend, measurements in backends.items():
            body += f'<section id="{esc(backend)}"><h2>{esc(backend)}</h2>'
            for kind, heading in (("daily", "Daily runs"), ("release", "Last five main releases")):
                points = ordered(measurements, kind)
                body += f'<h3>{heading}</h3>'
                if not points:
                    body += '<p class="note">No measurements available for this cadence.</p>'
                    continue
                reference_groups = groups(points, reference=True)
                for group in reference_groups.values():
                    body += cohort(points, group, reference=True)
                if not reference_groups:
                    body += '<p class="note">No equivalent-reference measurements for these points.</p>'
                for group in groups(points).values():
                    body += cohort(points, group)
                body += '<details><summary>Runs, status and raw data</summary><table><tr><th>Run</th><th>Status</th><th>Compiler</th></tr>'
                for event in points:
                    body += (f'<tr><td><a href="{event["href"]}">{esc(event["record"]["label"])}</a></td>'
                             f'<td>{esc(event["case"]["status"])}</td><td>{esc(event["case"].get("compiler", "—"))}</td></tr>')
                body += '</table></details>'
            body += '</section>'
        page(output / filename(name), name, body)
    return ('<h2>Subtests</h2><table><tr><th>Subtest</th><th>Family</th><th>Measured backends</th></tr>' +
            ''.join(rows) + '</table>')
