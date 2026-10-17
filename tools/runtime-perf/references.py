#!/usr/bin/env python3
"""Run fixed sensitive workloads against external toolchains in the same RHI harness."""
import argparse
import ctypes as C
import html
import json
import os
import re
from pathlib import Path
import shutil
import statistics
import subprocess
import time

import perf
from lib import sensitive_workloads as workloads


def nvrtc_compile(source, output, library, architecture):
    """Compile independently authored CUDA, without requiring NVCC's host toolchain."""
    library = Path(library).resolve()
    dll_directory = os.add_dll_directory(str(library.parent)) if os.name == "nt" else None
    try:
        api = C.CDLL(str(library))
        signatures = {
            "nvrtcVersion": [C.POINTER(C.c_int), C.POINTER(C.c_int)],
            "nvrtcCreateProgram": [C.POINTER(C.c_void_p), C.c_char_p, C.c_char_p,
                                   C.c_int, C.c_void_p, C.c_void_p],
            "nvrtcCompileProgram": [C.c_void_p, C.c_int, C.POINTER(C.c_char_p)],
            "nvrtcGetProgramLogSize": [C.c_void_p, C.POINTER(C.c_size_t)],
            "nvrtcGetProgramLog": [C.c_void_p, C.c_void_p],
            "nvrtcGetPTXSize": [C.c_void_p, C.POINTER(C.c_size_t)],
            "nvrtcGetPTX": [C.c_void_p, C.c_void_p],
            "nvrtcDestroyProgram": [C.POINTER(C.c_void_p)],
        }
        for name, types in signatures.items():
            getattr(api, name).argtypes = types
            getattr(api, name).restype = C.c_int

        def check(result):
            if result:
                raise RuntimeError(f"NVRTC error {result}")

        major, minor = C.c_int(), C.c_int()
        check(api.nvrtcVersion(C.byref(major), C.byref(minor)))
        program = C.c_void_p()
        check(api.nvrtcCreateProgram(C.byref(program), source.read_bytes(), b"reference.cu", 0, None, None))
        flags = [f"--gpu-architecture={architecture}", "--std=c++17", "--fmad=true"]
        try:
            options = (C.c_char_p * len(flags))(*(s.encode() for s in flags))
            start = time.perf_counter()
            result = api.nvrtcCompileProgram(program, len(flags), options)
            elapsed = (time.perf_counter() - start) * 1000
            size = C.c_size_t()
            check(api.nvrtcGetProgramLogSize(program, C.byref(size)))
            log = C.create_string_buffer(size.value)
            check(api.nvrtcGetProgramLog(program, log))
            output.with_suffix(".compile.log").write_bytes(log.value)
            check(result)
            check(api.nvrtcGetPTXSize(program, C.byref(size)))
            ptx = C.create_string_buffer(size.value)
            check(api.nvrtcGetPTX(program, ptx))
            output.write_bytes(ptx.raw)
        finally:
            check(api.nvrtcDestroyProgram(C.byref(program)))
        return dict(tool="NVRTC", version=f"{major.value}.{minor.value}",
                    tool_sha256=perf.digest(library.read_bytes()), flags=flags,
                    compile_observation_ms=elapsed, timing_scope="NVRTC compile API, one observation")
    finally:
        if dll_directory:
            dll_directory.close()


def compile_reference(backend, directory, args):
    language, extension, binary = {
        "vulkan": ("glsl", "comp", "spv"), "d3d12": ("hlsl", "hlsl", "dxil"),
        "cuda": ("cuda", "cu", "ptx")}[backend]
    source, output = directory / f"reference.{extension}", directory / f"reference.{binary}"
    if backend == "cuda":
        if not args.nvrtc or not args.cuda_arch:
            raise ValueError("CUDA reference requires --nvrtc and --cuda-arch matching the selected GPU")
        record = nvrtc_compile(source, output, args.nvrtc, args.cuda_arch)
    else:
        tool = shutil.which(args.glslang if backend == "vulkan" else args.dxc)
        if not tool:
            raise ValueError(f"missing {backend} reference compiler")
        flags = (["-V", "--target-env", "vulkan1.2", "-S", "comp", "-o", str(output), str(source)]
                 if backend == "vulkan" else
                 ["-T", "cs_6_6", "-E", "computeMain", "-O3", "-Fo", str(output), str(source)])
        version = subprocess.run([tool, "--version"], capture_output=True, text=True, timeout=30)
        command = [tool, *flags]
        start = time.perf_counter()
        process = subprocess.run(command, capture_output=True, timeout=120)
        elapsed = (time.perf_counter() - start) * 1000
        output.with_suffix(".compile.log").write_bytes(process.stdout + process.stderr)
        if process.returncode:
            raise RuntimeError(f"reference compilation failed: {output.with_suffix('.compile.log')}")
        record = dict(tool="glslang" if backend == "vulkan" else "DXC",
                      version=(version.stdout + version.stderr).strip(),
                      tool_sha256=perf.digest(Path(tool).read_bytes()), command=command,
                      compile_observation_ms=elapsed, timing_scope="compiler process wall time, one observation")
    record.update(source_sha256=perf.digest(source.read_bytes()),
                  binary_sha256=perf.digest(output.read_bytes()), language=language)
    return output, record


def execute(args, backend, case, directory, implementation, binary, batch, label):
    command = [str(Path(args.runner).resolve()), "--backend", backend, "--adapter", str(args.adapter),
               "--case", case, "--directory", str(directory), "--elements", str(args.elements),
               "--batch", str(batch), "--samples", str(args.samples), "--max-samples", str(args.samples),
               "--target-ms", str(args.target_ms)]
    if implementation == "reference":
        command += ["--reference-binary", str(binary)]
    process = subprocess.run(command, capture_output=True, text=True, timeout=args.timeout)
    (directory / f"{label}.stdout.json").write_text(process.stdout, encoding="utf-8")
    (directory / f"{label}.stderr.log").write_text(process.stderr, encoding="utf-8")
    if process.returncode not in (0, 2):
        raise RuntimeError(f"runner failed ({process.returncode}): {directory / (label + '.stderr.log')}")
    record = json.loads(process.stdout)
    if not record.get("validated"):
        raise ValueError("runner did not validate outputs")
    if implementation == "reference" and record.get("reference_cache_hits", 0) < 1:
        raise ValueError("reference binary was not executed")
    record.update(implementation=implementation, command=command, timers=perf.aggregate(record["samples"]),
                  target_code_sha256=perf.digest((directory / "main.target-code").read_bytes()))
    # Keep each compiled binary, including both implementations, beside its measurements.
    shutil.copyfile(directory / "main.target-code", directory / f"{label}.target-code")
    return record


def summarize(runs):
    samples = [s["gpu_iteration_ms"] for r in runs for s in r["samples"]]
    median = statistics.median(samples)
    round_medians = [r["timers"]["gpu_iteration_ms"]["median"] for r in runs]
    mad = statistics.median(abs(x - median) for x in samples) / median
    round_range = (max(round_medians) - min(round_medians)) / median
    return dict(median_ms=median, relative_mad=mad, round_range=round_range,
                valid=all(r["status"] == "ok" for r in runs) and mad <= 0.05 and round_range <= 0.05)


def comparison(baseline, current):
    if not baseline.get("complete") or not current.get("complete"):
        raise ValueError("incomplete measurements cannot be compared")
    for key in ("schema_version", "workload_version", "environment", "measurement", "runner_sources"):
        if baseline[key] != current[key]:
            raise ValueError(f"incompatible history: {key}")
    old = {(r["backend"], r["case"]): r for r in baseline["cases"]}
    if set(old) != {(r["backend"], r["case"]) for r in current["cases"]}:
        raise ValueError("incompatible history: coverage")
    comparisons = []
    for row in current["cases"]:
        before = old[row["backend"], row["case"]]
        for key in ("slang_source_sha256", "reference_source_sha256", "batch", "adapter", "rhi_revision"):
            if before[key] != row[key]:
                raise ValueError(f"incompatible history: {row['case']} {key}")
        for implementation in ("slang", "reference"):
            if not before["summary"][implementation]["valid"] or not row["summary"][implementation]["valid"]:
                raise ValueError("unstable measurements cannot be compared")
        # Freeze alternative toolchains as well as workloads for historical comparison.
        for key in ("tool_sha256", "source_sha256", "binary_sha256"):
            if before["reference_compilation"][key] != row["reference_compilation"][key]:
                raise ValueError(f"incompatible history: reference {key}")
        comparisons.append(dict(backend=row["backend"], case=row["case"],
                                ratio=row["summary"]["slang"]["median_ms"] / before["summary"]["slang"]["median_ms"]))
    return comparisons


def report(result, output):
    rows = []
    for row in result["cases"]:
        s, r = row["summary"]["slang"], row["summary"]["reference"]
        valid = s["valid"] and r["valid"]
        ratio = f"{s['median_ms'] / r['median_ms']:.3f}" if valid else "unstable"
        rows.append(f"<tr><td>{html.escape(row['backend'])}</td><td>{html.escape(row['case'])}</td>"
                    f"<td>{s['median_ms'] * 1000:.2f}</td><td>{r['median_ms'] * 1000:.2f}</td><td>{ratio}</td>"
                    f"<td>{100 * s['relative_mad']:.2f}% / {100 * r['relative_mad']:.2f}%</td>"
                    f"<td>{100 * s['round_range']:.2f}% / {100 * r['round_range']:.2f}%</td></tr>")
    history = ""
    if "history" in result:
        history = "<h2>Slang versus baseline " + html.escape(result.get("baseline_compiler", "")) + "</h2><ul>" + "".join(
            f"<li>{html.escape(r['backend'] + ' / ' + r['case'])}: {(r['ratio'] - 1) * 100:+.2f}%</li>"
            for r in result["history"]) + "</ul>"
    output.write_text("""<!doctype html><meta charset="utf-8"><title>Sensitive runtime prototype</title>
<style>body{font:16px system-ui;margin:40px;max-width:1400px;background:#f8fafc;color:#172033}
table{border-collapse:collapse;width:100%;background:white}td,th{padding:10px;text-align:left;border-bottom:1px solid #ddd}
th{background:#e5edf5}p{max-width:1050px;line-height:1.5}code{background:#e5edf5}</style>
<h1>Sensitive runtime workloads and references</h1>"""
        + ("<p><b>Run in progress or interrupted; results are incomplete.</b></p>" if not result.get("complete") else "")
        + f"<p>Compiler: <b>{html.escape(result['compiler'])}</b>. "
        + f"GPU: {html.escape(result['cases'][0]['adapter'])}. Driver: {html.escape(result['environment']['driver_id'])}.</p>"
        + "<p>Fixed, versioned kernels; identical resources, dispatches, correctness checks and RHI timers. "
        + "Vulkan reference: GLSL/glslang. D3D12: HLSL/DXC. CUDA: hand-written CUDA/NVRTC. "
        + "Material references flatten the generic abstractions. These are equivalent algorithm references, not tuned performance ceilings.</p>"
        + "<p>Times are microseconds per iteration of a serialized batch, including barriers and timestamp overhead. "
        + "Ratio = Slang/reference; below 1 is faster. MAD and round range show Slang/reference separately. "
        + "Rounds alternate implementation order. Cross-backend ratios are not compiler comparisons.</p>"
        + f"<p>Elements: {result['measurement']['elements']:,}; samples/round: {result['measurement']['samples']}; "
        + f"rounds: {result['measurement']['rounds']}; target batch duration: {result['measurement']['target_ms']} ms.</p>"
        + "<table><tr><th>Backend</th><th>Case</th><th>Slang µs</th><th>Reference µs</th><th>Ratio</th>"
        + "<th>Pooled MAD</th><th>Round range</th></tr>" + "".join(rows) + "</table>" + history
        + "<p>Compilation: raw JSON contains one-time reference compilation observations and Slang API preparation observations. "
        + "Their boundaries differ; they are not comparable compile-time benchmarks. Use perf.py --slangc for repeated paired Slang compile measurements.</p>"
        + '<p><a href="results.json">Raw results, commands, compiler identities and hashes</a></p>', encoding="utf-8")


def run(args):
    output = Path(args.output).resolve()
    if output.exists() and any(output.iterdir()):
        raise ValueError("output must be empty")
    if not args.driver_id or args.driver_id == "unknown":
        raise ValueError("provide an actual --driver-id")
    if args.elements < 65536 or args.elements > 16777216 or args.elements % 256:
        raise ValueError("elements must be a multiple of 256 in [65536,16777216]")
    if not 3 <= args.samples <= 1000 or not 2 <= args.rounds <= 20 or args.target_ms <= 0:
        raise ValueError("invalid sample/round/target settings")
    cases = list(workloads.CASES) if args.cases == "all" else args.cases.split(",")
    if set(cases) - workloads.CASES.keys():
        raise ValueError("unknown sensitive case")
    output.mkdir(parents=True, exist_ok=True)
    config = dict(perf.DEFAULTS, driver_id=args.driver_id, environment_id=args.environment_id,
                  backend=args.backends, adapter=args.adapter)
    result = dict(schema_version=1, workload_version=workloads.VERSION, complete=False,
                  environment=perf.environment(config), compiler="", created_unix=time.time(),
                  measurement=dict(elements=args.elements, samples=args.samples, rounds=args.rounds, target_ms=args.target_ms),
                  runner_sources={p.name: perf.digest(p.read_bytes()) for p in (perf.HERE / "native").glob("*.*")},
                  runner_sha256=perf.digest(Path(args.runner).read_bytes()), cases=[])
    baseline = json.loads(Path(args.baseline).read_text()) if args.baseline else None
    for backend in args.backends.split(","):
        if backend not in ("vulkan", "d3d12", "cuda"):
            raise ValueError("unsupported backend")
        for case in cases:
            print(f"[reference] {backend} {case}", flush=True)
            directory = output / backend / case
            directory.mkdir(parents=True)
            slang_source = workloads.generate(case)
            language, ext = {"vulkan": ("glsl", "comp"), "d3d12": ("hlsl", "hlsl"), "cuda": ("cuda", "cu")}[backend]
            reference_source = workloads.generate(case, language)
            (directory / "main.slang").write_text(slang_source)
            (directory / f"reference.{ext}").write_text(reference_source)
            binary, compilation = compile_reference(backend, directory, args)
            if baseline:
                previous = next(r for r in baseline["cases"] if r["backend"] == backend and r["case"] == case)
                batch = previous["batch"]
            else:
                pilots = [execute(args, backend, case, directory, impl, binary, 0, "calibrate-" + impl)
                          for impl in ("slang", "reference")]
                batch = max(r["batch"] for r in pilots)
            runs = {"slang": [], "reference": []}
            for round_index in range(args.rounds):
                order = ("slang", "reference") if round_index % 2 == 0 else ("reference", "slang")
                for impl in order:
                    runs[impl].append(execute(args, backend, case, directory, impl, binary, batch, f"r{round_index}-{impl}"))
            first = runs["slang"][0]
            if backend == "cuda":
                targets = [re.search(rb"\.target\s+(sm_\d+)", (directory / f"r0-{impl}.target-code").read_bytes())
                           for impl in ("slang", "reference")]
                if any(t is None for t in targets) or targets[0][1] != targets[1][1]:
                    raise ValueError("CUDA target architectures differ; set --cuda-arch to the Slang PTX target")
            if first["runner_source_sha256"] != perf.runner_source_hash():
                raise ValueError("rebuild runner from current sources")
            result["compiler"] = first["compiler"]
            row = dict(backend=backend, case=case, batch=batch, adapter=first["adapter"], rhi_revision=first["rhi_revision"],
                       slang_source_sha256=perf.digest(slang_source.encode()),
                       reference_source_sha256=perf.digest(reference_source.encode()),
                       reference_compilation=compilation, runs=runs, summary={k: summarize(v) for k, v in runs.items()})
            result["cases"].append(row)
            perf.write_json(output / "results.json", result)
            report(result, output / "report.html")
            print(f"[measured] {backend} {case}: " + str(row["summary"]), flush=True)
    result["complete"] = True
    if baseline:
        result["baseline_compiler"] = baseline["compiler"]
        result["history"] = comparison(baseline, result)
    perf.write_json(output / "results.json", result)
    report(result, output / "report.html")
    return 0 if all(s["valid"] for r in result["cases"] for s in r["summary"].values()) else 2


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--runner", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--driver-id", required=True)
    parser.add_argument("--environment-id", default="")
    parser.add_argument("--backends", default="vulkan,d3d12,cuda")
    parser.add_argument("--cases", default="all")
    parser.add_argument("--elements", type=int, default=4194304)
    parser.add_argument("--samples", type=int, default=20)
    parser.add_argument("--rounds", type=int, default=2)
    parser.add_argument("--target-ms", type=float, default=25)
    parser.add_argument("--timeout", type=int, default=300)
    parser.add_argument("--adapter", type=int, default=0)
    parser.add_argument("--dxc", default="dxc")
    parser.add_argument("--glslang", default="glslangValidator")
    parser.add_argument("--nvrtc")
    parser.add_argument("--cuda-arch", help="PTX architecture matching Slang, e.g. compute_89 for RTX 4090")
    parser.add_argument("--baseline", help="Previous daily/release results.json; freezes batches and compares Slang")
    args = parser.parse_args()
    return run(args)


if __name__ == "__main__":
    raise SystemExit(main())
