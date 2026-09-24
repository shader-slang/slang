#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""Assess named complex shaders through NVRTC O3 and NVVM O0/O3 without running kernels.

Exit 0 requires every compile and assembly to pass; 1 preserves shader/compiler failures;
2 indicates invalid inputs or missing tools. Failure latency is never a successful compile metric.
"""

import argparse
from datetime import datetime, timezone
import importlib.util
import json
import os
from pathlib import Path
import platform
import re
import statistics
import subprocess


REPO = Path(__file__).resolve().parents[1]
MODES = (("nvrtc", 3), ("nvvm", 0), ("nvvm", 3))
TIMER = re.compile(r"^\[\*\]\s+(\w+)\s+\d+\s+([\d.]+)ms", re.MULTILINE)


def load_toolkit_helpers():
    """Reuse the established timeout, log, hashing, and selected-toolkit contracts."""
    path = REPO / "extras/validate-nvvm-toolkit.py"
    spec = importlib.util.spec_from_file_location("nvvm_toolkit", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def read_workloads(path, toolkit):
    """Validate immutable source identities before invoking either compiler backend."""
    manifest = json.loads(path.read_text(encoding="utf-8"))
    if manifest.get("schema") != 1 or manifest.get("scope") != "compile-and-assemble-only":
        raise ValueError("expected schema 1 compile-and-assemble-only corpus")
    architecture = manifest["architecture"]
    if not isinstance(architecture, int) or architecture < 10 or architecture > 999:
        raise ValueError("invalid manifest architecture")
    workloads = manifest["workloads"]
    if not isinstance(workloads, list) or not workloads:
        raise ValueError("manifest has no workloads")
    names = set()
    for workload in workloads:
        name = workload["name"]
        if not re.fullmatch(r"[a-zA-Z0-9_-]+", name) or name in names:
            raise ValueError("invalid or duplicate workload name: " + name)
        names.add(name)
        source = (REPO / workload["source"]).resolve()
        if not source.is_relative_to(REPO):
            raise ValueError("workload source must be inside the repository")
        toolkit.require_file(source)
        if toolkit.sha256(source) != workload["source_sha256"]:
            raise ValueError("source hash mismatch: " + workload["source"])
        entries = workload["entry_points"]
        if not isinstance(entries, list) or not entries or len(entries) != len(set(entries)):
            raise ValueError("empty or duplicate entry-point list: " + name)
        if any(not re.fullmatch(r"[a-zA-Z_]\w*", entry) for entry in entries):
            raise ValueError("invalid entry-point name: " + name)
        if workload["stage"] != "compute":
            raise ValueError("complex NVVM corpus requires compute entry points")
    return manifest


def assess_cell(cell, workload, args, toolkit, environment):
    """Record every attempt and require fresh PTX plus assembly for a passing cell."""
    directory = args.output / cell["id"]
    directory.mkdir(parents=True, exist_ok=True)
    command = [
        str(args.slangc), str(REPO / workload["source"]),
        "-entry", cell["entry"], "-stage", workload["stage"], "-target", "ptx",
        "-capability", f"cuda_sm_{cell['architecture'] // 10}_{cell['architecture'] % 10}",
        f"-O{cell['optimization']}", f"-emit-cuda-via-{cell['backend']}",
        "-report-perf-benchmark",
    ]
    cell["attempts"] = []
    cell["successful_compile_median_seconds"] = None
    cell["phase_median_ms"] = None
    for index in range(args.warmup + args.samples):
        ptx = directory / f"attempt-{index}.ptx"
        ptx.unlink(missing_ok=True)
        log = directory / f"attempt-{index}.log"
        attempt = toolkit.run(command + ["-o", str(ptx)], log, environment, args.timeout)
        attempt["warmup"] = index < args.warmup
        text = log.read_text(encoding="utf-8", errors="replace") if log.exists() else ""
        attempt["phase_ms"] = {name: float(value) for name, value in TIMER.findall(text)}
        cell["attempts"].append(attempt)
        if attempt["return_code"] != 0:
            cell["status"] = (
                "infrastructure-failed" if attempt["return_code"] is None else
                "preflight-rejected" if "error[E52017]" in text else "compile-failed"
            )
            cell["diagnostics"] = [line for line in text.splitlines()
                                   if "error[" in line or "error :" in line or "error:" in line]
            # Further samples of a rejected shader would only measure failure latency.
            return
        toolkit.require_file(ptx)
        ptx_text = ptx.read_text(encoding="utf-8")
        if not re.search(r"^\s*\.target\s+sm_" + str(cell["architecture"]) + r"\b", ptx_text, re.M):
            raise ValueError("PTX target mismatch: " + str(ptx))
        if not re.search(r"\.entry\s+" + re.escape(cell["entry"]) + r"\s*\(", ptx_text):
            raise ValueError("PTX entry point missing: " + str(ptx))
        attempt["ptx_sha256"] = toolkit.sha256(ptx)
        attempt["ptx_bytes"] = ptx.stat().st_size
    measured = [attempt for attempt in cell["attempts"] if not attempt["warmup"]]
    cell["successful_compile_median_seconds"] = statistics.median(
        attempt["elapsed_seconds"] for attempt in measured
    )
    shared_timers = set.intersection(*(set(attempt["phase_ms"]) for attempt in measured))
    cell["phase_median_ms"] = {
        name: statistics.median(attempt["phase_ms"][name] for attempt in measured)
        for name in sorted(shared_timers)
    }
    cubin = directory / "out.cubin"
    cubin.unlink(missing_ok=True)
    assembly = toolkit.run(
        [str(args.ptxas), "-v", f"-arch=sm_{cell['architecture']}", str(ptx), "-o", str(cubin)],
        directory / "ptxas.log", environment, args.timeout,
    )
    cell["assembly"] = assembly
    if assembly["return_code"] != 0:
        cell["status"] = "assembly-failed"
        return
    toolkit.require_file(cubin)
    cell.update(status="passed", ptx=str(ptx), ptx_bytes=ptx.stat().st_size,
                cubin=str(cubin), cubin_bytes=cubin.stat().st_size,
                cubin_sha256=toolkit.sha256(cubin))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--slangc", type=Path, required=True)
    parser.add_argument("--build-label", required=True, help="e.g. Debug; required timing provenance")
    parser.add_argument("--provider", type=Path, required=True)
    parser.add_argument("--cuda-root", type=Path, required=True)
    parser.add_argument("--manifest", type=Path, default=Path(__file__).with_name("complex-corpus.manifest.json"))
    parser.add_argument("--output", type=Path, default=REPO / "build/nvvm-complex")
    parser.add_argument("--samples", type=int, default=3)
    parser.add_argument("--warmup", type=int, default=1)
    parser.add_argument("--timeout", type=int, default=180)
    args = parser.parse_args()
    if args.samples < 1 or args.warmup < 0 or args.timeout < 1:
        parser.error("samples and timeout must be positive; warmup must be nonnegative")
    for name in ("slangc", "provider", "cuda_root", "manifest", "output"):
        setattr(args, name, getattr(args, name).resolve())
    args.output.mkdir(parents=True, exist_ok=True)
    results = args.output / "results.json"
    report = {"schema": 1, "status": "infrastructure-failed", "gpu_execution": False,
              "started_utc": datetime.now(timezone.utc).isoformat(),
              "build_label": args.build_label, "platform": platform.platform(), "cells": []}

    def save():
        results.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")

    save()
    try:
        toolkit = load_toolkit_helpers()
        manifest = read_workloads(args.manifest, toolkit)
        args.ptxas = args.cuda_root / "bin" / ("ptxas.exe" if os.name == "nt" else "ptxas")
        nvcc = args.cuda_root / "bin" / ("nvcc.exe" if os.name == "nt" else "nvcc")
        for path in (args.slangc, args.provider, args.ptxas, nvcc):
            toolkit.require_file(path)
        libnvvm = toolkit.select_libnvvm(args.cuda_root)
        libdevice = toolkit.require_file(args.cuda_root / "nvvm/libdevice/libdevice.10.bc")
        if os.name == "nt":
            nvrtcs = list((args.cuda_root / "bin").glob("nvrtc64_*.dll"))
            if len(nvrtcs) != 1:
                raise ValueError("expected one NVRTC DLL in selected CUDA bin directory")
            nvrtc = nvrtcs[0]
        else:
            nvrtc = toolkit.require_file(args.cuda_root / "lib64/libnvrtc.so").resolve()
        environment = dict(os.environ, CUDA_PATH=str(args.cuda_root), CUDA_HOME=str(args.cuda_root),
                           LIBNVVM_HOME=str(args.cuda_root), SLANG_NVVM_BUILDER_PATH=str(args.provider))
        variable = "PATH" if os.name == "nt" else "LD_LIBRARY_PATH"
        environment[variable] = os.pathsep.join(
            [str(libnvvm.parent), str(nvrtc.parent), environment.get(variable, "")])
        report.update(
            manifest=manifest, compiler=str(args.slangc), provider=str(args.provider),
            samples=args.samples, warmup=args.warmup,
            compiler_version=subprocess.check_output([str(args.slangc), "-version"], env=environment,
                                                    stderr=subprocess.STDOUT, text=True, timeout=args.timeout).strip(),
            toolkit_version=subprocess.check_output([str(nvcc), "--version"], env=environment,
                                                   text=True, timeout=args.timeout).strip(),
            artifact_sha256={str(path): toolkit.sha256(path)
                             for path in (args.slangc, args.provider, libnvvm, libdevice, nvrtc)},
        )
        for workload in manifest["workloads"]:
            for entry in workload["entry_points"]:
                for backend, optimization in MODES:
                    cell = {"id": f"{workload['name']}-{entry}-{backend}-o{optimization}",
                            "workload": workload["name"], "entry": entry, "backend": backend,
                            "optimization": optimization, "architecture": manifest["architecture"],
                            "status": "pending"}
                    report["cells"].append(cell)
        report["status"] = "running"
        save()
        workloads = {workload["name"]: workload for workload in manifest["workloads"]}
        for cell in report["cells"]:
            try:
                assess_cell(cell, workloads[cell["workload"]], args, toolkit, environment)
            except (OSError, ValueError) as error:
                cell.update(status="infrastructure-failed", error=str(error))
            print(f"{cell['id']}: {cell['status']}", flush=True)
            save()
        report["status"] = "passed" if all(cell["status"] == "passed" for cell in report["cells"]) else "incomplete"
        save()
        return 0 if report["status"] == "passed" else 1
    except (OSError, ValueError, KeyError, TypeError, subprocess.SubprocessError) as error:
        report.update(status="infrastructure-failed", error=str(error))
        save()
        print(str(error))
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
