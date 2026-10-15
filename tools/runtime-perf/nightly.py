#!/usr/bin/env python3
"""Collect runtime bundles for the existing compile-perf nightly publication."""
import argparse
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import time

import perf


def coverage(backend):
    # OptiX is not part of the current suite. CUDA still requires every compute case.
    return [name for name, family in perf.workloads.CASES.items()
            if backend != "cuda" or family != "raytracing"]


def collect_references(args, output):
    """Measure equivalent implementations after the primary daily measurements."""
    command = [sys.executable, str(perf.HERE / "references.py"),
               "--runner", str(Path(args.runner).resolve()), "--output", str(output / "references"),
               "--backends", args.backends, "--driver-id", args.driver_id,
               "--environment-id", args.environment_id]
    if "cuda" in args.backends.split(","):
        libraries = list(Path(os.environ.get("CUDA_PATH", "__missing_cuda__")).glob("bin/**/nvrtc64_*.dll"))
        if not libraries:
            raise ValueError("CUDA_PATH must identify the CUDA toolkit containing NVRTC")
        target = output / "cuda/sources/sensitive_material_32/main.target-code"
        match = re.search(rb"\.target\s+sm_(\d+)", target.read_bytes())
        if not match:
            raise ValueError("cannot determine the measured Slang CUDA architecture")
        command += ["--nvrtc", str(sorted(libraries)[-1]), "--cuda-arch", "compute_" + match[1].decode()]
    with (output / "references.log").open("w", encoding="utf-8") as log:
        process = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT, timeout=args.timeout)
    return dict(status="ok" if process.returncode == 0 else "failed", exit_code=process.returncode,
                command=command)


def baseline(results, config, corpus, rhi_revision=None):
    """Reuse the newest compatible calibration, never silently resize a candidate."""
    candidates = []
    for path in results.glob("daily/*/runtime/*/*/results.json"):
        try:
            data = json.loads(path.read_text(encoding="utf-8"))
            if not isinstance(data, dict):
                continue
            candidates.append((data.get("created_unix", 0), path, data))
        except (OSError, ValueError):
            continue
    for _, path, data in sorted(candidates, reverse=True):
        try:
            if {r["workload"] for r in data["runs"]} != set(corpus):
                continue
            if any(r["status"] != "ok" or not r.get("validated") for r in data["runs"]):
                continue
            if rhi_revision and any(r.get("rhi_revision") != rhi_revision for r in data["runs"]):
                continue
            calibration = path.with_name("calibration.json")
            perf.check_calibration(json.loads(calibration.read_text(encoding="utf-8")),
                                   perf.environment(config), config, corpus)
            return calibration
        except (OSError, ValueError, KeyError, TypeError):
            continue
    return None


def collect(args):
    for value in (args.label, args.attempt):
        if not re.fullmatch(r"[A-Za-z0-9_.-]+", value) or value in (".", ".."):
            raise ValueError("label and attempt must be single path components")
    root = Path(args.results).resolve()
    output = root / "daily" / args.label / "runtime" / args.attempt
    output.mkdir(parents=True, exist_ok=False)
    record = dict(schema_version=1, label=args.label, attempt=args.attempt,
                  commit=args.commit, suite_commit=args.suite_commit,
                  created_unix=time.time(), status="running", backends={})
    manifest = output / "run.json"
    perf.write_json(manifest, record)
    try:
        if args.failure:
            raise ValueError(args.failure)
        if not args.driver_id:
            raise ValueError("GPU driver identity is required")
        rhi_revision = subprocess.check_output(
            ["git", "-C", str(perf.HERE.parents[1] / "external/slang-rhi"), "rev-parse", "HEAD"],
            text=True, timeout=30).strip()
        record["rhi_revision"] = rhi_revision
        for backend in args.backends.split(","):
            if backend not in ("vulkan", "d3d12", "cuda") or backend in record["backends"]:
                raise ValueError("invalid or duplicate backend")
            cases = coverage(backend)
            config = dict(perf.DEFAULTS, backend=backend, cases=",".join(cases),
                          driver_id=args.driver_id, environment_id=args.environment_id,
                          samples=args.samples, max_samples=max(30, args.samples))
            corpus = {case: {"source_sha256": perf.source_hash(
                perf.workloads.generate(case, config["complexity"]))} for case in cases}
            saved = baseline(root, config, corpus, rhi_revision)
            item = dict(status="running", expected_cases=cases,
                        baseline=str(saved.relative_to(root)) if saved else None)
            record["backends"][backend] = item
            perf.write_json(manifest, record)
            command = [sys.executable, str(perf.HERE / "perf.py"), "run",
                       "--runner", str(Path(args.runner).resolve()),
                       "--slangc", str(Path(args.slangc).resolve()),
                       "--backend", backend, "--cases", config["cases"],
                       "--samples", str(config["samples"]),
                       "--max-samples", str(config["max_samples"]),
                       "--driver-id", args.driver_id, "--environment-id", args.environment_id,
                       "--output", str(output / backend)]
            if saved:
                command += ["--calibration", str(saved)]
            item["command"] = command
            try:
                with (output / (backend + ".log")).open("w", encoding="utf-8") as log:
                    process = subprocess.run(command, stdout=log, stderr=subprocess.STDOUT,
                                             timeout=args.timeout)
                item["exit_code"] = process.returncode
                data = json.loads((output / backend / "results.json").read_text(encoding="utf-8"))
                complete = {r["workload"] for r in data["runs"]} == set(cases)
                valid = complete and all(r["status"] == "ok" and r.get("validated") for r in data["runs"])
                item["status"] = "ok" if process.returncode == 0 and valid else "failed"
                print(f"{backend}: {item['status']}", flush=True)
            except (OSError, ValueError, KeyError, subprocess.TimeoutExpired) as error:
                item.update(status="failed", reason=str(error))
            perf.write_json(manifest, record)
        if args.references:
            try:
                record["references"] = collect_references(args, output)
            except (OSError, ValueError, subprocess.TimeoutExpired) as error:
                record["references"] = dict(status="failed", reason=str(error))
        record["status"] = "ok" if (all(v["status"] == "ok" for v in record["backends"].values())
                                      and record.get("references", {}).get("status", "ok") == "ok") else "failed"
    except (OSError, ValueError, subprocess.SubprocessError) as error:
        record.update(status="failed", reason=str(error))
    finally:
        perf.write_json(manifest, record)
        summary = (f"## GPU runtime — {record['status']}\n\n"
                   f"Compiler commit: `{args.commit}`. Attempt: `{args.attempt}`.\n\n"
                   "| Backend | Status | Required cases |\n|---|---|---:|\n" +
                   "".join(f"| {backend} | {item['status']} | {len(item['expected_cases'])} |\n"
                           for backend, item in record["backends"].items()))
        if record.get("reason"):
            summary += "\n" + record["reason"] + "\n"
        if "references" in record:
            summary += f"\nEquivalent references: {record['references']['status']}.\n"
        summary += "\nTiming changes are advisory. See the archived run bundle for raw samples and paired CLI compile times.\n"
        (output / "summary.md").write_text(summary, encoding="utf-8")
        if os.environ.get("GITHUB_STEP_SUMMARY"):
            with open(os.environ["GITHUB_STEP_SUMMARY"], "a", encoding="utf-8") as stream:
                stream.write(summary)
    return 0 if record["status"] == "ok" else 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--results", required=True)
    parser.add_argument("--label", required=True)
    parser.add_argument("--attempt", required=True)
    parser.add_argument("--commit", required=True)
    parser.add_argument("--suite-commit", required=True)
    parser.add_argument("--runner", default="")
    parser.add_argument("--slangc", default="")
    parser.add_argument("--driver-id", default="")
    parser.add_argument("--environment-id", default="")
    parser.add_argument("--backends", default="vulkan,d3d12,cuda")
    parser.add_argument("--samples", type=int, default=20)
    parser.add_argument("--references", action="store_true", help="Also collect equivalent native references")
    parser.add_argument("--timeout", type=int, default=7200)
    parser.add_argument("--failure", help="Archive a setup failure without attempting GPU execution")
    return collect(parser.parse_args())


if __name__ == "__main__":
    raise SystemExit(main())
