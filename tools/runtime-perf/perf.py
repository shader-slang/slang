#!/usr/bin/env python3
"""Portable, offline runtime-perf prototype. Python 3.9+, standard library only."""
import argparse
import dataclasses
import hashlib
import json
import math
import os
from pathlib import Path
import platform
import socket
import statistics
import subprocess
import sys
import time

HERE = Path(__file__).resolve().parent
COMPILE_PERF = HERE.parent / "compile-perf"
sys.path.insert(0, str(COMPILE_PERF))
import bench
from lib import manifest, runtime_workloads as workloads

DEFAULTS = dict(backend="vulkan", adapter=0,
                complexity=4, samples=10, max_samples=30, warmup=5, target_ms=25.0,
                max_spread=0.05, strict_phases=False,
                max_elements=16777216, elements=0, batch=0, max_batch=1024, timeout=300,
                cases="all", environment_id="", driver_id="", allow_unsupported=False,
                allow_unstable=False, compile_samples=3)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def write_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, allow_nan=False) + "\n", encoding="utf-8")


def source_hash(files):
    return digest(json.dumps(files, sort_keys=True, ensure_ascii=True).encode())


def selected_cases(selection):
    if selection == "all":
        return list(workloads.CASES)
    result = []
    for item in selection.split(","):
        matches = [case for case, family in workloads.CASES.items() if item in (case, family)]
        if not matches:
            raise ValueError(f"unknown workload or family: {item}")
        result.extend(case for case in matches if case not in result)
    return result


def settings(args):
    config = json.loads(Path(args.config).read_text()) if args.config else {}
    unknown = set(config) - set(DEFAULTS)
    if unknown:
        raise ValueError(f"unknown configuration fields: {sorted(unknown)}")
    result = dict(DEFAULTS, **config)
    for key in DEFAULTS:
        value = getattr(args, key, None)
        if value is not None:
            result[key] = value
    if result["backend"] not in ("metal", "vulkan", "cuda", "d3d12"):
        raise ValueError("unsupported backend name")
    for key in ("allow_unstable", "allow_unsupported", "strict_phases"):
        if type(result[key]) is not bool:
            raise ValueError(f"{key} must be a boolean")
    for key in ("cases", "environment_id", "driver_id"):
        if not isinstance(result[key], str):
            raise ValueError(f"{key} must be a string")
    for key in ("adapter", "complexity", "samples", "max_samples", "warmup", "max_elements", "elements", "batch", "max_batch", "compile_samples"):
        if type(result[key]) is not int:
            raise ValueError(f"{key} must be an integer")
    if not 1 <= result["complexity"] <= 64:
        raise ValueError("complexity must be between 1 and 64")
    if not 3 <= result["samples"] <= result["max_samples"] <= 1000:
        raise ValueError("require 3 <= samples <= max_samples <= 1000")
    if not 1 <= result["warmup"] <= 1000 or result["adapter"] < 0 or result["compile_samples"] < 1:
        raise ValueError("invalid warmup, adapter, or compile_samples")
    if not 1 <= result["max_batch"] <= 1024 or not 0 <= result["batch"] <= result["max_batch"]:
        raise ValueError("require 0 <= batch <= max_batch <= 1024, max_batch >= 1")
    for key in ("target_ms", "max_spread", "timeout"):
        if not isinstance(result[key], (int, float)) or not math.isfinite(result[key]) or result[key] <= 0:
            raise ValueError(f"{key} must be finite and positive")
    if not 65536 <= result["max_elements"] <= 16777216 or result["max_elements"] % 256:
        raise ValueError("max_elements must be a multiple of 256 in [65536, 16777216]")
    if result["elements"] and (not 65536 <= result["elements"] <= result["max_elements"] or result["elements"] % 256):
        raise ValueError("elements must be zero or a multiple of 256 within the configured limits")
    return result


def environment(config):
    # Metal's driver is delivered with the OS. Other backends require an
    # explicit driver identifier before cross-run comparison is permitted.
    driver = config["driver_id"]
    if not driver and config["backend"] == "metal" and sys.platform == "darwin":
        driver = "macOS " + platform.mac_ver()[0] + " " + platform.version()
    return dict(host=socket.gethostname(), os=platform.platform(), machine=platform.machine(),
                environment_id=config["environment_id"], driver_id=driver,
                backend=config["backend"], adapter_index=config["adapter"])


def policy(config):
    return {key: config[key] for key in ("complexity", "warmup", "samples", "max_samples", "target_ms", "max_spread", "strict_phases")}


def prepare(output, cases, complexity):
    corpus = {}
    for case in cases:
        files = workloads.generate(case, complexity)
        directory = output / "sources" / case
        directory.mkdir(parents=True, exist_ok=True)
        for name, source in files.items():
            (directory / name).write_bytes(source.encode("ascii"))
        corpus[case] = dict(directory=str(directory.resolve()), source_sha256=source_hash(files), files=list(files))
    return corpus


def aggregate(samples):
    if not samples:
        raise ValueError("runner returned no measurements")
    keys = set(samples[0])
    if any(set(sample) != keys for sample in samples):
        raise ValueError("inconsistent sample phases")
    for sample in samples:
        if any(not isinstance(v, (int, float)) or not math.isfinite(v) or v < 0 for v in sample.values()):
            raise ValueError("invalid timing sample")
    return {key: bench.stats([sample[key] for sample in samples]) for key in sorted(keys)}


def check_calibration(saved, env, config, corpus):
    if saved.get("schema_version") != 1 or saved.get("workload_version") != workloads.VERSION:
        raise ValueError("incompatible calibration schema/workload version")
    if saved["environment"] != env or saved["policy"] != policy(config):
        raise ValueError("calibration environment or measurement policy differs")
    if saved.get("runner_source_sha256") != digest((HERE / "native/runner.cpp").read_bytes()):
        raise ValueError("calibration runner implementation differs")
    for case, source in corpus.items():
        entry = saved["cases"].get(case)
        if not entry or entry["source_sha256"] != source["source_sha256"]:
            raise ValueError(f"calibration source mismatch: {case}")


def compile_case(slangc, case, config, source, output):
    records = []
    target = dict(metal="metal", vulkan="spirv", cuda="ptx", d3d12="dxil")[config["backend"]]
    for filename in source["files"]:
        stage = Path(filename).stem
        name = "runtime_" + case + ("_" + stage if case == "image_pipeline" else "")
        spec = dataclasses.replace(manifest.BY_NAME[name], extra_flags=["-target", target, "-profile", "sm_6_6", "-O3", "-matrix-layout-row-major"],
                                   downstream_required=True)
        record = bench.run_spec(str(slangc), spec, config["complexity"], config["compile_samples"], 1,
                                str(output / "compile-sources"), str(output / "compile-artifacts"))
        records.append(record)
    return records


def run(args):
    config = settings(args)
    cases = selected_cases(config["cases"])
    output = Path(args.output).resolve()
    # A result bundle is immutable: accidental reuse must not mix two runs.
    if output.exists() and any(output.iterdir()):
        raise ValueError(f"output directory is not empty: {output}")
    output.mkdir(parents=True, exist_ok=True)
    corpus = prepare(output, cases, config["complexity"])
    env = environment(config)
    saved = json.loads(Path(args.calibration).read_text()) if args.calibration else None
    if saved:
        check_calibration(saved, env, config, corpus)
    runner = Path(args.runner).resolve()
    if not runner.is_file():
        raise ValueError(f"runner not found: {runner}")
    compiler_version = None
    if args.slangc:
        compiler_version = subprocess.run([str(Path(args.slangc).resolve()), "-version"], text=True,
                                       capture_output=True, check=True, timeout=30)
        compiler_version = (compiler_version.stdout + compiler_version.stderr).strip()
    results = dict(schema_version=1, workload_version=workloads.VERSION, environment=env,
                   config=config, created_unix=time.time(), runner_sha256=digest(runner.read_bytes()),
                   generator_sha256=digest(Path(workloads.__file__).read_bytes()),
                   runner_source_sha256=digest((HERE / "native/runner.cpp").read_bytes()), runs=[])
    calibration = dict(schema_version=1, workload_version=workloads.VERSION, environment=env,
                       policy=policy(config), runner_source_sha256=results["runner_source_sha256"], cases={})
    failed = False
    for case in cases:
        source = corpus[case]
        command = [str(runner), "--case", case, "--directory", source["directory"]]
        selected = dict(config)
        if saved:
            selected["elements"] = saved["cases"][case]["elements"]
            selected["batch"] = saved["cases"][case]["batch"]
        for key in ("backend", "adapter", "complexity", "elements", "max_elements", "batch", "max_batch", "samples", "max_samples", "warmup", "target_ms", "max_spread"):
            command.extend(["--" + key.replace("_", "-"), str(selected[key])])
        command.extend(["--strict-phases", str(int(selected["strict_phases"]))])
        print(f"[run] {case}", flush=True)
        try:
            process = subprocess.run(command, text=True, capture_output=True, timeout=config["timeout"])
            (output / f"{case}.stderr.log").write_text(process.stderr, encoding="utf-8")
            (output / f"{case}.stdout.log").write_text(process.stdout, encoding="utf-8")
            record = json.loads(process.stdout) if process.stdout.strip() else {}
            if process.returncode not in (0, 2, 77) or record.get("status") not in ("ok", "unstable", "unsupported"):
                raise ValueError(f"runner failed ({process.returncode}); see {case}.stderr.log")
            if record["status"] != "unsupported":
                if record.get("runner_source_sha256") != results["runner_source_sha256"]:
                    raise ValueError("runner binary was built from different sources; rebuild it")
                if not record.get("validated"):
                    raise ValueError("runner did not validate output")
                record["timers"] = aggregate(record["samples"])
                record["relative_mad_by_phase"] = {}
                for name, stat in record["timers"].items():
                    if name.startswith("gpu_"):
                        center = statistics.median(stat["samples"])
                        record["relative_mad_by_phase"][name] = (
                            statistics.median(abs(v - center) for v in stat["samples"]) / center if center > 0 else 1.0)
                record["noisy_phases"] = [
                    name for name, spread in record["relative_mad_by_phase"].items()
                    if name not in ("gpu_total_ms", "gpu_iteration_ms") and spread > config["max_spread"]]
                record["target_code_sha256"] = {
                    path.name: digest(path.read_bytes()) for path in Path(source["directory"]).glob("*.target-code")}
                if len(record["target_code_sha256"]) != len(source["files"]):
                    raise ValueError("runner did not archive every stage's target code")
                if compiler_version and compiler_version != record["compiler"]:
                    raise ValueError(f"slangc version {compiler_version} differs from runner compiler {record['compiler']}")
                if saved and (record["adapter"] != saved["cases"][case]["adapter"] or
                              record["rhi_revision"] != saved["cases"][case]["rhi_revision"]):
                    raise ValueError("calibration adapter or RHI revision differs")
                calibration["cases"][case] = {key: record[key] for key in ("elements", "batch", "adapter", "rhi_revision")}
                calibration["cases"][case]["source_sha256"] = source["source_sha256"]
                if args.slangc:
                    record["compile_runs"] = compile_case(Path(args.slangc).resolve(), case, config, source, output)
                    if any(not r["ok"] or not r["got_timers"] for r in record["compile_runs"]):
                        raise ValueError("paired compile measurement failed")
            if record["status"] == "unsupported" and not config["allow_unsupported"]:
                failed = True
            if record["status"] == "unstable" and not config["allow_unstable"]:
                failed = True
        except (ValueError, KeyError, TypeError, subprocess.TimeoutExpired, OSError) as error:
            record = dict(status="failed", reason=str(error))
            failed = True
        record.update(workload=case, family=workloads.CASES[case], size=config["complexity"],
                      source_sha256=source["source_sha256"], command=command)
        results["runs"].append(record)
        write_json(output / "results.json", results)
        write_json(output / "calibration.json", calibration)
        print(f"[{record['status']}] {case}" + (f" — {record['reason']}" if "reason" in record else ""), flush=True)
        if record.get("noisy_phases"):
            print("  Phase variability exceeds target: " + ", ".join(record["noisy_phases"]), flush=True)
    return 1 if failed else 0


def compare(left, right):
    a, b = (json.loads(Path(path).read_text()) for path in (left, right))
    if not a["environment"].get("driver_id") or not b["environment"].get("driver_id"):
        raise ValueError("comparison requires a recorded driver ID; supply --driver-id on run")
    for field in ("schema_version", "workload_version", "environment", "generator_sha256", "runner_source_sha256"):
        if a[field] != b[field]:
            raise ValueError(f"incompatible {field}")
    if policy(a["config"]) != policy(b["config"]):
        raise ValueError("incompatible measurement policies")
    old = {r["workload"]: r for r in a["runs"]}
    new = {r["workload"]: r for r in b["runs"]}
    if old.keys() != new.keys():
        raise ValueError("different workload coverage")
    comparisons = []
    for name, current in new.items():
        previous = old[name]
        if previous["status"] != "ok" or current["status"] != "ok":
            raise ValueError(f"{name}: comparison requires two stable, valid measurements")
        if not previous.get("validated") or not current.get("validated"):
            raise ValueError(f"{name}: missing correctness validation")
        for key in ("size", "elements", "width", "batch", "measurement_mode", "source_sha256", "adapter", "backend", "rhi_revision", "timestamp_frequency"):
            if previous[key] != current[key]:
                raise ValueError(f"{name}: incompatible {key}")
        p = previous["timers"]["gpu_total_ms"]["median"]
        c = current["timers"]["gpu_total_ms"]["median"]
        if not math.isfinite(p) or not math.isfinite(c) or p <= 0 or c <= 0:
            raise ValueError(f"{name}: nonpositive or nonfinite duration")
        if previous["timers"].keys() != current["timers"].keys():
            raise ValueError(f"{name}: different phase coverage")
        phases = {}
        for phase in current["timers"]:
            before = previous["timers"][phase]["median"]
            after = current["timers"][phase]["median"]
            if not math.isfinite(before) or not math.isfinite(after) or before < 0 or after < 0:
                raise ValueError(f"{name}: invalid {phase} duration")
            phases[phase] = dict(baseline_ms=before, candidate_ms=after,
                                 ratio=after / before if before > 0 else None)
            if phase.startswith("gpu_"):
                phases[phase]["above_spread_target"] = (
                    phase in previous.get("noisy_phases", []) or phase in current.get("noisy_phases", []))
        comparisons.append(dict(workload=name, baseline_ms=p, candidate_ms=c, ratio=c / p, phases=phases))
    return comparisons


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    sub.add_parser("list")
    devices = sub.add_parser("devices")
    devices.add_argument("--runner", required=True)
    prep = sub.add_parser("prepare")
    prep.add_argument("--output", required=True)
    prep.add_argument("--cases", default="all")
    prep.add_argument("--complexity", type=int, default=4)
    measure = sub.add_parser("run")
    measure.add_argument("--runner", required=True)
    measure.add_argument("--output", required=True)
    measure.add_argument("--config")
    measure.add_argument("--calibration", help="Reuse a baseline calibration; never resize the candidate")
    measure.add_argument("--slangc", help="Also measure identical generated sources using compile-perf")
    for key, value in DEFAULTS.items():
        flag = "--" + key.replace("_", "-")
        if isinstance(value, bool):
            measure.add_argument(flag, action="store_true", default=None)
        else:
            measure.add_argument(flag, type=type(value), default=None)
    comparison = sub.add_parser("compare")
    comparison.add_argument("baseline")
    comparison.add_argument("candidate")
    args = parser.parse_args()
    try:
        if args.command == "list":
            print(json.dumps(workloads.CASES, indent=2))
        elif args.command == "devices":
            return subprocess.run([str(Path(args.runner).resolve()), "--devices"]).returncode
        elif args.command == "prepare":
            print(json.dumps(prepare(Path(args.output), selected_cases(args.cases), args.complexity), indent=2))
        elif args.command == "run":
            return run(args)
        elif args.command == "compare":
            print(json.dumps(compare(args.baseline, args.candidate), indent=2))
    except (ValueError, OSError, KeyError, subprocess.SubprocessError) as error:
        parser.exit(1, f"runtime-perf: {error}\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
