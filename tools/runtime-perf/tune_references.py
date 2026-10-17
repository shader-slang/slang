#!/usr/bin/env python3
"""Explore reference optimizations separately from equivalent-implementation tracking.

The winner is exploratory. Freeze its source and confirm independently before
promoting it to a long-term performance target; minimum selection is biased.
"""
import argparse
import html
import json
import os
from pathlib import Path

import perf
import references
from lib import sensitive_workloads as workloads


def report(result, output):
    equivalent_report = html.escape(os.path.relpath(Path(result["baseline"]).with_name("report.html"), output.parent).replace(os.sep, "/"), quote=True)
    rows = []
    for row in result["cases"]:
        summaries = row["summary"]
        slang, equivalent = summaries["slang"], summaries["equivalent"]
        valid = {k: v for k, v in summaries.items() if k != "slang" and v["valid"]}
        winner = min(valid, key=lambda k: valid[k]["median_ms"]) if valid else None
        tuned = valid[winner]["median_ms"] if winner else None
        ratio = f"{slang['median_ms'] / tuned:.3f}" if tuned and slang["valid"] else "unstable"
        rows.append(f"<tr><td>{html.escape(row['backend'])}</td><td>{html.escape(row['case'])}</td>"
                    f"<td>{slang['median_ms'] * 1000:.2f}</td><td>{equivalent['median_ms'] * 1000:.2f}</td>"
                    f"<td>{tuned * 1000:.2f}</td><td>{html.escape(winner)}</td><td>{ratio}</td></tr>"
                    if winner else f"<tr><td>{html.escape(row['backend'])}</td><td>{html.escape(row['case'])}</td><td colspan=5>No stable reference</td></tr>")
    details = []
    for row in result["cases"]:
        details.append(f"<h3>{html.escape(row['backend'] + ' / ' + row['case'])}</h3><ul>" + "".join(
            f"<li>{html.escape(name)}: {s['median_ms'] * 1000:.2f} µs; MAD {100*s['relative_mad']:.2f}%; "
            f"round range {100*s['round_range']:.2f}%; {'valid' if s['valid'] else 'unstable'}</li>"
            for name, s in row["summary"].items()) + "</ul>")
    output.write_text("""<!doctype html><meta charset="utf-8"><title>Equivalent and tuned references</title>
<style>body{font:16px system-ui;margin:40px;max-width:1400px;color:#172033;background:#f8fafc}
p{max-width:1100px;line-height:1.5}table{border-collapse:collapse;width:100%;background:white}
td,th{padding:10px;text-align:left;border-bottom:1px solid #ddd}th{background:#e5edf5}</style>
<h1>Equivalent implementations and tuned performance targets</h1>"""
        + ("<p><b>Incomplete tuning run.</b></p>" if not result["complete"] else "")
        + f"<p>Slang {html.escape(result['compiler'])}; {html.escape(result['environment']['driver_id'])} driver. "
        + "Same workload, input, precision tolerance, dispatch dimensions, batch size, and timing boundaries for every variant.</p>"
        + "<p><b>Equivalent</b> compares compiler/language handling of the same algorithm. "
        + "<b>Best measured reference</b> gives an empirical target from explicit optimization candidates. "
        + "It is not a proven optimum. Selection is exploratory: confirm the winner in independent runs before adopting it as a regression target.</p>"
        + "<p>Unroll adds static-loop unrolling (and CUDA read-only loads). Unroll-affine also folds the two material layers "
        + "into coefficients, reassociating FP32 arithmetic within the existing correctness tolerance. "
        + "Every candidate validates all outputs before and after timing. Implementation order rotates across rounds.</p>"
        + "<table><tr><th>Backend</th><th>Case</th><th>Slang µs</th><th>Equivalent µs</th>"
        + "<th>Best reference µs</th><th>Reference variant</th><th>Slang / best</th></tr>"
        + "".join(rows) + "</table><h2>Candidate measurements</h2>" + "".join(details)
        + '<p><a href="results.json">Raw samples, source/binary hashes and compiler commands</a></p>'
        + f'<p><a href="{equivalent_report}">Full equivalent-implementation suite</a></p>', encoding="utf-8")


def run(args):
    baseline = json.loads(Path(args.baseline).read_text())
    if not baseline.get("complete"):
        raise ValueError("reference tuning requires a complete equivalent baseline")
    if baseline["runner_sources"] != {p.name: perf.digest(p.read_bytes()) for p in (perf.HERE / "native").glob("*.*")}:
        raise ValueError("harness differs from equivalent baseline")
    output = Path(args.output).resolve()
    if output.exists() and any(output.iterdir()):
        raise ValueError("output must be empty")
    config = dict(perf.DEFAULTS, driver_id=args.driver_id, environment_id=args.environment_id,
                  backend=baseline["environment"]["backend"], adapter=args.adapter)
    if perf.environment(config) != baseline["environment"]:
        raise ValueError("environment differs from equivalent baseline")
    args.elements = baseline["measurement"]["elements"]
    args.target_ms = baseline["measurement"]["target_ms"]
    output.mkdir(parents=True)
    result = dict(schema_version=1, complete=False, compiler=baseline["compiler"],
                  environment=baseline["environment"], baseline=str(Path(args.baseline).resolve()),
                  measurement=dict(elements=args.elements, target_ms=args.target_ms, samples=args.samples, rounds=args.rounds), cases=[])
    selected = [r for r in baseline["cases"] if (args.cases == "all" or r["case"] in args.cases.split(","))
                and r["backend"] in args.backends.split(",")]
    if not selected:
        raise ValueError("no matching baseline cases")
    for base in selected:
        case, backend = base["case"], base["backend"]
        print(f"[tune] {backend} {case}", flush=True)
        variants = ["equivalent", "unroll"]
        if workloads.CASES[case][0] == "material":
            variants.append("unroll-affine")
        items = {}
        language, extension = {"vulkan": ("glsl", "comp"), "d3d12": ("hlsl", "hlsl"), "cuda": ("cuda", "cu")}[backend]
        for variant in variants:
            directory = output / backend / case / variant
            directory.mkdir(parents=True)
            source = workloads.generate(case)
            if perf.digest(source.encode()) != base["slang_source_sha256"]:
                raise ValueError("Slang workload changed")
            (directory / "main.slang").write_text(source)
            (directory / f"reference.{extension}").write_text(workloads.generate(case, language, variant))
            binary, compilation = references.compile_reference(backend, directory, args)
            if variant == "equivalent":
                for key in ("tool_sha256", "source_sha256", "binary_sha256"):
                    if compilation[key] != base["reference_compilation"][key]:
                        raise ValueError(f"equivalent reference changed: {key}")
            items[variant] = dict(directory=directory, binary=binary, compilation=compilation, runs=[])
        items["slang"] = dict(directory=items["equivalent"]["directory"], binary=items["equivalent"]["binary"], runs=[])
        for index in range(args.rounds):
            order = list(items)
            order = order[index % len(order):] + order[:index % len(order)]
            for name in order:
                item = items[name]
                record = references.execute(args, backend, case, item["directory"],
                                            "slang" if name == "slang" else "reference", item["binary"],
                                            base["batch"], f"r{index}-{name}")
                if record["compiler"] != baseline["compiler"] or record["adapter"] != base["adapter"]:
                    raise ValueError("compiler or adapter changed during tuning")
                if record["runner_source_sha256"] != perf.runner_source_hash() or record["rhi_revision"] != base["rhi_revision"]:
                    raise ValueError("runner or RHI differs from equivalent baseline")
                item["runs"].append(record)
        row = dict(backend=backend, case=case, batch=base["batch"],
                   summary={k: references.summarize(v["runs"]) for k, v in items.items()},
                   candidates={k: {field: value for field, value in v.items() if field not in ("directory", "binary")}
                               for k, v in items.items()})
        result["cases"].append(row)
        perf.write_json(output / "results.json", result)
        report(result, output / "report.html")
        print(row["summary"], flush=True)
    result["complete"] = True
    perf.write_json(output / "results.json", result)
    report(result, output / "report.html")
    return 0 if all(s["valid"] for r in result["cases"] for s in r["summary"].values()) else 2


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    for key in ("runner", "baseline", "output", "driver-id"):
        parser.add_argument("--" + key, required=True)
    parser.add_argument("--environment-id", default="")
    parser.add_argument("--adapter", type=int, default=0)
    parser.add_argument("--cases", default="sensitive_material_32")
    parser.add_argument("--backends", default="vulkan,d3d12,cuda")
    parser.add_argument("--samples", type=int, default=20)
    parser.add_argument("--rounds", type=int, default=4)
    parser.add_argument("--timeout", type=int, default=300)
    parser.add_argument("--nvrtc")
    parser.add_argument("--cuda-arch")
    parser.add_argument("--dxc", default="dxc")
    parser.add_argument("--glslang", default="glslangValidator")
    args = parser.parse_args()
    if not 3 <= args.samples <= 1000 or not 2 <= args.rounds <= 20:
        parser.error("require 3..1000 samples and 2..20 rounds")
    return run(args)


if __name__ == "__main__":
    raise SystemExit(main())
