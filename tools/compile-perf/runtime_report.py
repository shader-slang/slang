"""Runtime section of the shared performance report; no GPU needed to render it."""
import html
import json
import math
from pathlib import Path
import sys
import zipfile

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "runtime-perf"))
import perf as runtime_perf
import runtime_charts


def esc(value):
    return html.escape(str(value), quote=True)


def number(value, scale=1):
    return f"{value * scale:.3f}" if isinstance(value, (int, float)) and math.isfinite(value) else "—"


def page(path, title, body):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text('<!doctype html><html lang="en"><meta charset="utf-8">'
                   '<meta name="viewport" content="width=device-width,initial-scale=1">'
                   f'<title>{esc(title)}</title><style>'
                   'body{font:15px/1.5 system-ui;margin:32px;color:#172b40;background:#fafbfc}'
                   'main{max-width:1450px;margin:auto}table{border-collapse:collapse;width:100%;background:white}'
                   'td,th{padding:8px 12px;text-align:left;border-bottom:1px solid #dae0e7}'
                   'th{background:#eaf0f7}a{color:#075da8}pre{white-space:pre-wrap;overflow-wrap:anywhere}'
                   '.scroll{overflow:auto}.chart{max-width:1100px;background:white;border:1px solid #dae0e7;margin:16px 0}'
                   '.chart svg{display:block;width:100%;height:auto}.note{color:#536375}h2{margin-top:32px}'
                   '</style><main><p><a href="../index.html">Performance overview</a> · '
                   '<a href="index.html">GPU runtime</a></p>'
                   f'<h1>{esc(title)}</h1>{body}</main></html>', encoding="utf-8")


def archive(directory, destination):
    """Keep published bundles byte-stable across fresh Git checkouts."""
    with zipfile.ZipFile(destination, "w", compression=zipfile.ZIP_DEFLATED) as bundle:
        for path in sorted(directory.rglob("*")):
            if path.is_file():
                entry = zipfile.ZipInfo(path.relative_to(directory).as_posix())
                entry.compress_type = zipfile.ZIP_DEFLATED
                entry.external_attr = 0o644 << 16
                bundle.writestr(entry, path.read_bytes())


def previous_case(history, bundle, case, label):
    """Find a prior valid daily point using exactly the runtime comparison contract."""
    current = dict(bundle, runs=[case])
    reasons = []
    for old_label, old_bundle in reversed(history):
        if old_label == label:
            continue
        old = next((r for r in old_bundle.get("runs", []) if r["workload"] == case["workload"]), None)
        if old is None:
            continue
        try:
            comparison = runtime_perf.compare_results(dict(old_bundle, runs=[old]), current)[0]
            return f"{(comparison['ratio'] - 1) * 100:+.2f}%", old_label
        except (ValueError, KeyError, TypeError) as error:
            reasons.append(str(error))
    return "—", reasons[0] if reasons else "No previous compatible daily measurement"


def case_table(bundle, expected, history, label):
    rows = []
    runs = {r["workload"]: r for r in bundle.get("runs", [])}
    for name in expected:
        case = runs.get(name, dict(workload=name, status="missing"))
        timers = case.get("timers", {})
        delta, baseline = previous_case(history, bundle, case, label)
        spread = case.get("relative_mad_by_phase", {}).get("gpu_total_ms")
        rows.append('<tr>' + ''.join(f'<td>{cell}</td>' for cell in (
            esc(name), esc(case.get("family", "—")), esc(case["status"]),
            number(timers.get("gpu_iteration_ms", {}).get("median"), 1000),
            number(timers.get("gpu_total_ms", {}).get("median")),
            number(spread, 100), esc(delta), esc(baseline),
            esc(case.get("elements", "—")), esc(case.get("batch", "—")))) + '</tr>')
    return ('<div class="scroll"><table><thead><tr>' + ''.join(f'<th>{s}</th>' for s in
            ("Case", "Family", "Status", "µs / execution", "Batch ms", "MAD %", "Daily Δ",
             "Baseline / comparison unavailable", "Elements", "Batch")) +
            '</tr></thead><tbody>' + ''.join(rows) + '</tbody></table></div>')


def details(bundle):
    parts = []
    for case in bundle.get("runs", []):
        parts.append(f'<details><summary>{esc(case["workload"])}: phases, compilation and provenance</summary>')
        if case.get("reason"):
            parts.append(f'<p>{esc(case["reason"])}</p>')
        parts.append('<table><tr><th>Repeated timer</th><th>Median ms</th><th>Samples</th></tr>')
        for name, stat in case.get("timers", {}).items():
            parts.append(f'<tr><td>{esc(name)}</td><td>{number(stat.get("median"))}</td>'
                         f'<td>{len(stat.get("samples", []))}</td></tr>')
        parts.append('</table><h3>Paired CLI compilation of executed sources</h3>')
        for compile_run in case.get("compile_runs", []):
            parts.append(f'<p>{esc(compile_run.get("workload", "shader"))} — '
                         f'CLI wall time: {number((compile_run.get("wall_ms") or {}).get("median"))} ms</p><table>')
            for name, stat in compile_run.get("timers", {}).items():
                if isinstance(stat, dict):
                    parts.append(f'<tr><td>{esc(name)}</td><td>{number(stat.get("median"))}</td></tr>')
            parts.append('</table>')
        diagnostic = {k: v for k, v in case.items() if k not in ("timers", "samples", "compile_runs")}
        parts.append('<h3>Setup observations and provenance</h3><pre>' +
                     esc(json.dumps(diagnostic, indent=2)) + '</pre></details>')
    return ''.join(parts)


def reference_table(path, status):
    body = '<h2>Equivalent implementation references</h2>'
    body += (f'<p>Status: {esc(status["status"])}. Ratio is Slang / equivalent; below 1 is faster. '
             'These comparisons have their own matched dimensions and alternating measurement rounds. '
             'No independently confirmed optimized target has been frozen for nightly use yet.</p>')
    try:
        data = json.loads(path.read_text(encoding="utf-8"))
        body += '<table><tr><th>Backend</th><th>Case</th><th>Slang µs</th><th>Equivalent µs</th><th>Ratio</th><th>Status</th></tr>'
        for row in data["cases"]:
            slang, reference = row["summary"]["slang"], row["summary"]["reference"]
            valid = data.get("complete") and slang["valid"] and reference["valid"]
            ratio = number(slang["median_ms"] / reference["median_ms"]) if valid else "—"
            body += '<tr>' + ''.join(f'<td>{esc(value)}</td>' for value in (
                row["backend"], row["case"], number(slang["median_ms"], 1000),
                number(reference["median_ms"], 1000), ratio, "ok" if valid else "incomplete / unstable")) + '</tr>'
        return body + '</table>'
    except (OSError, ValueError, KeyError, ZeroDivisionError) as error:
        return body + '<p>Reference results unavailable: ' + esc(status.get("reason", str(error))) + '</p>'


def write_report(results, outdir):
    root, output = Path(results), Path(outdir) / "runtime"
    output.mkdir(parents=True, exist_ok=True)
    manifests, errors = [], []
    for path in list(root.glob("daily/*/runtime/*/run.json")) + list(root.glob("releases/*/runtime/*/run.json")):
        try:
            record = json.loads(path.read_text(encoding="utf-8"))
            if not isinstance(record, dict) or record.get("schema_version") != 1:
                raise ValueError("unsupported run manifest schema")
            for key in ("label", "attempt", "commit", "status"):
                if not isinstance(record[key], str):
                    raise ValueError(f"invalid manifest {key}")
            if not isinstance(record["backends"], dict) or not math.isfinite(record["created_unix"]):
                raise ValueError("invalid manifest backends or timestamp")
            record["kind"] = "release" if path.relative_to(root).parts[0] == "releases" else "daily"
            manifests.append((record, path))
        except (OSError, ValueError, KeyError, TypeError) as error:
            errors.append(f"{path.relative_to(root)}: {error}")
    manifests.sort(key=lambda entry: entry[0]["created_unix"])
    history, links = {}, []
    chart_events = []
    for record, path in manifests:
        identity = runtime_perf.digest(str(path.relative_to(root)).encode())[:20]
        stem = "run-" + identity
        # Publish the complete reproducible bundle, not machine-local file links.
        archive(path.parent, output / (stem + ".zip"))
        title = f"{record['label']} / attempt {record['attempt']}"
        reference_data = {}
        try:
            reference_data = json.loads((path.parent / "references/results.json").read_text(encoding="utf-8"))
            if not isinstance(reference_data, dict) or not isinstance(reference_data.get("cases"), list):
                reference_data = {}
        except (OSError, ValueError):
            pass
        body = (f'<p>Status: <b>{esc(record["status"])}</b> · compiler commit '
                f'<code>{esc(record["commit"])}</code></p>'
                f'<p><a href="{stem}.zip">Download sources, samples, binaries and logs</a></p>')
        if record.get("reason"):
            body += f'<p>{esc(record["reason"])}</p>'
        if record.get("provenance"):
            body += f'<p>{esc(record["provenance"])}</p>'
        for backend, item in record["backends"].items():
            body += f'<h2>{esc(backend)} — {esc(item["status"])}</h2>'
            try:
                bundle = json.loads((path.parent / backend / "results.json").read_text(encoding="utf-8"))
                if bundle.get("schema_version") != 2:
                    raise ValueError("unsupported runtime results schema")
                table = case_table(bundle, item["expected_cases"], history.get(backend, []), record["label"])
                body += table + details(bundle)
                if record["kind"] == "daily":
                    history.setdefault(backend, []).append((record["label"], bundle))
                by_case = {r["workload"]: r for r in bundle.get("runs", [])}
                for name in item["expected_cases"]:
                    reference = next((r for r in reference_data.get("cases", [])
                                      if isinstance(r, dict) and r.get("backend") == backend and r.get("case") == name), None)
                    chart_events.append(dict(id=identity, record=record, kind=record["kind"],
                                             backend=backend, bundle=bundle,
                                             case=by_case.get(name, dict(workload=name, status="missing")),
                                             href=stem + ".html", references=reference_data, reference=reference))
            except (OSError, ValueError, KeyError, TypeError) as error:
                message = esc(item.get("reason", str(error)))
                body += f'<p>Measurement unavailable: {message}</p>'
                for name in item.get("expected_cases", []):
                    chart_events.append(dict(id=identity, record=record, kind=record["kind"],
                                             backend=backend, bundle={},
                                             case=dict(workload=name, status="missing"),
                                             href=stem + ".html", references={}, reference=None))
        if "references" in record:
            reference_body = reference_table(path.parent / "references/results.json", record["references"])
            reference_body += f'<p>Run: <a href="{stem}.html">{esc(title)}</a></p>'
            body += reference_body
        page(output / (stem + ".html"), title, body)
        links.append(f'<li><a href="{stem}.html">{esc(title)}</a> — {esc(record["status"])}</li>')
    intro = ('<p>Choose a subtest to see daily and main-release graphs for GPU execution and paired compilation. '
             'Slang and equivalent-reference timings share a graph when measured at matching dimensions. '
             'Timing changes remain advisory.</p>'
             '<p class="note">µs / execution is serialized batch throughput with reused resources, '
             'not isolated dispatch latency. MAD describes sample variability, not a confidence interval. '
             'GPU and host intervals overlap. Setup and ray-scene construction are excluded from GPU timings.</p>')
    if not manifests:
        intro += '<p>No runtime nightlies have been collected. Existing compile-only history is unchanged.</p>'
    if manifests and manifests[-1][0].get("reason"):
        intro += '<p>Latest attempt failed before collection: ' + esc(manifests[-1][0]["reason"]) + '</p>'
    intro += runtime_charts.write_subtests(output, chart_events, page)
    intro += '<details><summary>Run archive and measurement health</summary><ul>' + ''.join(reversed(links)) + '</ul></details>'
    if errors:
        intro += '<h2>Unreadable history</h2><pre>' + esc('\n'.join(errors)) + '</pre>'
    page(output / "index.html", "GPU runtime performance", intro)
    return len(manifests)


if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--results", required=True)
    args = parser.parse_args()
    write_report(args.results, Path(args.results) / "analysis")
