#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Reproduce NVVM checkpoints, repeated material timings, quality observations and reports.

Every execution requires a new output directory. A nonzero exit leaves its evidence intact.
Runtime acceptance compares exact identities and all five outcome fields, not runner exits alone.
"""

import argparse
import collections
from datetime import datetime, timezone
import hashlib
import importlib.util
import json
import math
import os
from pathlib import Path
import platform
import re
import signal
import shutil
import statistics
import subprocess
import sys
import time
from types import SimpleNamespace

REPO = Path(__file__).resolve().parents[1]
HERE = Path(__file__).resolve().parent
FIELDS = ("classification", "return_code", "execution_counts", "diagnostic", "canonical_shape")
MODES = ("nvrtc-o3", "nvvm-o0", "nvvm-o3")


def read(path):
    return json.loads(Path(path).read_text())


def write(path, value):
    Path(path).write_text(json.dumps(value, indent=2) + "\n")


def sha(path):
    digest = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def reference(path):
    return {"path": str(Path(path).resolve()), "sha256": sha(path)}


def load_runner():
    spec = importlib.util.spec_from_file_location("complex_runner", HERE / "run-complex-corpus.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def load_surface_results():
    """Load the physical proof comparator shared by checkpoint and CPU contracts."""
    spec = importlib.util.spec_from_file_location("surface_results", HERE / "nvvm-surface-results.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def run(command, log, environment, timeout):
    """Time fresh process creation through exit; write captured output outside the timer.

    Kill the process group on timeout so a runner cannot leave competing compiler children alive.
    Keep the failed command and log even when no output artifact was produced.
    """
    command = list(map(str, command))
    row = {"command": command, "log": str(log), "return_code": None, "timed_out": False}
    started = time.perf_counter()
    try:
        process = subprocess.Popen(command, cwd=REPO, env=environment, stdout=subprocess.PIPE,
                                   stderr=subprocess.STDOUT, start_new_session=True)
        try:
            output, _ = process.communicate(timeout=timeout)
        except subprocess.TimeoutExpired:
            row["timed_out"] = True
            os.killpg(process.pid, signal.SIGKILL)
            output, _ = process.communicate()
        row["return_code"] = process.returncode
    except OSError as error:
        output = str(error).encode()
        row["error"] = str(error)
    row["elapsed_seconds"] = time.perf_counter() - started
    Path(log).write_bytes(output)
    return row


def require_success(row):
    if row["return_code"] != 0 or row["timed_out"]:
        raise ValueError("failed process; inspect " + row["log"])


def index_outcomes(rows):
    """Reject ambiguous cells and false passes before comparing preservation obligations."""
    result = {}
    for row in rows:
        key = (row["id"], row["mode"])
        if key in result or row["mode"] not in MODES:
            raise ValueError("duplicate or invalid cell: " + str(key))
        value = {field: row[field] for field in FIELDS}
        counts = value["execution_counts"]
        if value["classification"] == "correct" and (
            value["return_code"] != 0 or counts != {"passed": 1, "executed": 1,
                                                        "ignored": 0, "other_summary_status": 0}
        ):
            raise ValueError("correct cell has invalid execution counts: " + str(key))
        result[key] = value
    if not result:
        raise ValueError("empty outcome inventory")
    return result


def compare_rows(old, new, allow_additions=False):
    before, after = index_outcomes(old), index_outcomes(new)
    missing, additions = sorted(before.keys() - after.keys()), sorted(after.keys() - before.keys())
    transitions = [{"id": key[0], "mode": key[1], "before": before[key], "after": after[key]}
                   for key in sorted(before.keys() & after.keys()) if before[key] != after[key]]
    incomplete = [key for key in after if any((key[0], mode) not in after for mode in MODES)]
    return {"status": "passed" if not (missing or transitions or incomplete
                                         or (additions and not allow_additions)
                                         or any(after[key]["classification"] != "correct"
                                                for key in additions)) else "review-required",
            "previous_cells": len(before), "observed_cells": len(after),
            "preserved_cells": len(before.keys() & after.keys()) - len(transitions),
            "missing": missing, "additions": additions, "incomplete_modes": incomplete,
            "transitions": transitions, "counts": dict(collections.Counter(
                row["classification"] for row in new))}


def accepted_baseline(path):
    baseline = read(path)
    if baseline.get("status") != "accepted-full":
        raise ValueError("baseline must be explicitly reviewed accepted-full; preserve unresolved comparisons")
    return baseline


def compare(args):
    baseline = accepted_baseline(args.baseline)
    result = {"schema": 1, "kind": "comparison", "baseline": reference(args.baseline),
              "corpora": {}, "status": "passed"}
    compact = {"schema": 1, "status": "comparison-passed", "baseline": reference(args.baseline),
               "corpora": {}, "runtime_input_sha256": baseline.get("runtime_input_sha256", {}),
               "unresolved_failures": baseline.get("unresolved_failures", []),
               "resolved_failure_history": baseline.get("resolved_failure_history", [])}
    for name in ("frozen", "discovery"):
        path = getattr(args, name)
        rows = read(path)
        delta = compare_rows(baseline["corpora"][name]["fresh_cell_outcomes"], rows,
                             args.allow_additions)
        result["corpora"][name] = delta
        compact["corpora"][name] = {"raw_results": reference(path), "fresh_cell_outcomes": [
            {key: row[key] for key in ("id", "mode") + FIELDS} for row in rows]}
        if delta["status"] != "passed":
            result["status"] = compact["status"] = "review-required"
    write(args.output / "comparison.json", result)
    write(args.output / "outcomes.json", compact)
    return result


def configure(args, output):
    """Pin selected libraries and record source, executable, input and device provenance."""
    runner = load_runner()
    toolkit = runner.load_toolkit_helpers()
    args.slangc = args.slangc.resolve()
    args.provider = args.provider.resolve()
    args.cuda_root = args.cuda_root.resolve()
    libnvvm = toolkit.select_libnvvm(args.cuda_root)
    nvrtc = toolkit.require_file(args.cuda_root / "lib64/libnvrtc.so").resolve()
    ptxas = toolkit.require_file(args.cuda_root / "bin/ptxas")
    libdevice = toolkit.require_file(args.cuda_root / "nvvm/libdevice/libdevice.10.bc")
    environment = dict(os.environ, CUDA_PATH=str(args.cuda_root), CUDA_HOME=str(args.cuda_root),
                       LIBNVVM_HOME=str(args.cuda_root), SLANG_NVVM_BUILDER_PATH=str(args.provider),
                       SLANG_NVVM_TEST_ARCH="80")
    environment["LD_LIBRARY_PATH"] = os.pathsep.join([
        str(args.slangc.parent), str(args.slangc.parent.parent / "lib"),
        str(libnvvm.parent), str(nvrtc.parent),
        environment.get("LD_LIBRARY_PATH", "")])
    paths = {args.slangc, args.provider, libnvvm, nvrtc, libdevice, ptxas,
             Path(__file__).resolve(), HERE / "run-complex-corpus.py",
             HERE / "nvvm-surface-results.py", REPO / "extras/validate-nvvm-surfaces.py"}
    # slangc dynamically loads these libraries and its builtin cache. Identify actual bytes.
    paths.update(args.slangc.parent.glob("*.so*"))
    paths.update(args.slangc.parent.glob("*.bin"))
    paths.update(path for path in (args.slangc.parent.parent / "lib").glob("*.so*")
                 if not path.name.endswith(".dwarf"))
    paths.update((args.slangc.parent.parent / "lib").glob("*.bin"))
    # Installed standard modules may live in versioned namespace subdirectories.
    for directory in (args.slangc.parent, args.slangc.parent.parent / "lib"):
        paths.update(directory.rglob("*.slang-module"))
    paths.add(args.slangc.parent / "test-server")
    if args.command == "quality":
        if (args.cuda_root / "bin/cuobjdump").is_file():
            paths.add(args.cuda_root / "bin/cuobjdump")
        if shutil.which("readelf"):
            paths.add(Path(shutil.which("readelf")).resolve())
    paths.update(args.slangc.parent.glob("slang-test*"))
    # Freeze the small tool/runtime inventory before adding the exhaustive source/input map.
    runtime_paths = tuple(paths)
    # Fresh surface fixtures can be untracked while a corpus extension is being reviewed.
    surface_harness = load_surface_results().load_harness()
    paths.update(REPO / "tests/cuda" / ("nvvm-surface-physical-" + row["fixture"] + ".slang")
                 for row in surface_harness.cases())
    tracked = subprocess.check_output(["git", "ls-files", "source", "include", "prelude",
                                       "external", "CMakeLists.txt", "cmake", "tests", "issue-nvvm-backend/*.py",
                                       "issue-nvvm-backend/*manifest*", "extras/*nvvm*.py"], cwd=REPO, text=True)
    paths.update(REPO / path for path in tracked.splitlines() if (REPO / path).is_file())
    provenance = {"started_utc": datetime.now(timezone.utc).isoformat(),
                  "revision": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=REPO,
                                                       text=True).strip(),
                  "platform": platform.platform(), "build_label": args.build_label,
                  "environment": {key: environment.get(key) for key in (
                      "CUDA_PATH", "CUDA_HOME", "LIBNVVM_HOME", "SLANG_NVVM_BUILDER_PATH",
                      "LD_LIBRARY_PATH", "CUDA_VISIBLE_DEVICES", "SLANG_NVVM_TEST_ARCH")},
                  "artifact_sha256": {str(path): sha(path) for path in sorted(paths)}}
    provenance["runtime_artifact_sha256"] = {
        str(path): provenance["artifact_sha256"][str(path)] for path in sorted(runtime_paths)}
    for name, command in (
        ("working-tree", ["git", "status", "--porcelain"]),
        ("source-diff", ["git", "diff", "HEAD", "--", "source", "include", "prelude", "external"]),
        ("submodules", ["git", "submodule", "status", "--recursive"]),
        ("compiler-version", [args.slangc, "-version"]),
        ("toolkit-version", [args.cuda_root / "bin/nvcc", "--version"]),
        ("ptxas-version", [ptxas, "--version"]),
        ("device", ["nvidia-smi", "--query-gpu=name,uuid,driver_version,compute_cap", "--format=csv"]),
    ):
        row = run(command, output / (name + ".log"), environment, 60)
        provenance[name] = row
        if name != "device":
            require_success(row)
    write(output / "provenance.json", provenance)
    return runner, environment, provenance


def verify_identity(provenance):
    changed = [path for path, digest in provenance["artifact_sha256"].items()
               if not Path(path).is_file() or sha(path) != digest]
    if changed:
        raise ValueError("inputs changed during execution: " + str(changed))


def cells_for(manifest, runner):
    return [{"id": f"{workload['name']}-{entry}-{backend}-o{opt}",
             "workload": workload["name"], "entry": entry, "backend": backend,
             "optimization": opt, "architecture": manifest["architecture"]}
            for workload in manifest["workloads"] for entry in workload["entry_points"]
            for backend, opt in runner.MODES]


def expected_inventory(ids, kind):
    """Keep the fixed slice257 protocol authoritative for both execution and acceptance."""
    rounds, attempts = (2, 11) if kind == "material" else (1, 1)
    samples = [(name, rd, index, kind == "material" and index < 2)
               for rd in range(rounds) for name in (ids if rd == 0 else list(reversed(ids)))
               for index in range(attempts)]
    assembly = [(name, index, kind == "material" and index < 2)
                for name in ids for index in range(attempts)]
    return samples, assembly


def validate_measurements(report, check_artifacts=True):
    """Reject partial, reordered, failed or mutated measurements before reporting medians."""
    ids = [cell["id"] for cell in report["cells"]]
    expected_cells = cells_for(report["manifest"], load_runner())
    if ids != [cell["id"] for cell in expected_cells]:
        raise ValueError("measurement cells differ from manifest")
    if not ids or len(ids) != len(set(ids)):
        raise ValueError("missing or duplicate measurement cells")
    samples, assembly = expected_inventory(ids, report["kind"])
    observed = [(row["id"], row["round"], row["index"], row["warmup"])
                for row in report["samples"]]
    assembled = [(row["id"], row["index"], row["warmup"]) for row in report["assembly"]]
    if samples != observed or assembly != assembled:
        raise ValueError("measurement inventory differs from the requested protocol")
    for rows, artifact in ((report["samples"], "ptx"), (report["assembly"], "cubin")):
        for row in rows:
            require_success(row)
            if not math.isfinite(row["elapsed_seconds"]) or row["elapsed_seconds"] <= 0 \
                    or not row.get(artifact + "_sha256"):
                raise ValueError("missing measurement output or duration")
            if check_artifacts and sha(row[artifact]) != row[artifact + "_sha256"]:
                raise ValueError("measurement artifact changed: " + row[artifact])
    for name in ids:
        for rows, key in ((report["samples"], "ptx_sha256"),
                          (report["assembly"], "cubin_sha256")):
            if len({row[key] for row in rows if row["id"] == name}) != 1:
                raise ValueError("non-deterministic output: " + name)
        for row in report["samples"]:
            if any(not math.isfinite(value) or value < 0 for value in row["phase_ms"].values()):
                raise ValueError("invalid phase duration")
        phases = [set(row["phase_ms"]) for row in report["samples"] if row["id"] == name]
        if not phases[0] or any(value != phases[0] for value in phases):
            raise ValueError("missing or inconsistent phase timers: " + name)


def parse_resources(text):
    """Preserve each ptxas function block; absent metrics remain unavailable, never zero."""
    functions = []
    for match in re.finditer(r"Function properties for (\S+)(.*?)(?=Function properties for |\Z)",
                             text, re.S):
        body = match.group(2)
        row = {"function": match.group(1)}
        for name, pattern in (
            ("registers", r"Used (\d+) registers"),
            ("stack_bytes", r"(\d+) bytes stack frame"),
            ("spill_store_bytes", r"(\d+) bytes spill stores"),
            ("spill_load_bytes", r"(\d+) bytes spill loads"),
            ("shared_bytes", r"(\d+) bytes smem"),
        ):
            value = re.search(pattern, body)
            row[name] = int(value.group(1)) if value else None
        functions.append(row)
    return functions


def measure(args):
    runner, environment, provenance = configure(args, args.output)
    manifest = runner.read_workloads(args.manifest, runner.load_toolkit_helpers())
    provenance["artifact_sha256"].update({str(REPO / w["source"]): w["source_sha256"]
                                         for w in manifest["workloads"]})
    provenance["artifact_sha256"][str(args.manifest.resolve())] = sha(args.manifest)
    if args.command == "quality":
        correctness = accepted_baseline(args.correctness)
        runtime_rows = [row for corpus in correctness["corpora"].values()
                        for row in corpus["fresh_cell_outcomes"]]
        indexed = index_outcomes(runtime_rows)
        for workload in manifest["workloads"]:
            for mode in MODES:
                if indexed[(workload["runtime_id"], mode)]["classification"] != "correct":
                    raise ValueError("quality fixture lacks accepted runtime correctness")
            if correctness["runtime_input_sha256"][workload["source"]] != workload["source_sha256"]:
                raise ValueError("quality fixture differs from correctness checkpoint")
        old_artifacts = correctness["provenance"]["artifact_sha256"]
        for artifact in (args.slangc, args.provider,
                         args.slangc.parent.parent / "lib/libslang-compiler.so"):
            expected = old_artifacts.get(str(artifact))
            if expected is None and artifact.is_relative_to(REPO):
                expected = old_artifacts.get(str(artifact.relative_to(REPO)))
            if expected != sha(artifact):
                raise ValueError("quality compiler differs from correctness checkpoint: " + str(artifact))
        provenance["correctness"] = reference(args.correctness)
    write(args.output / "provenance.json", provenance)
    cells = cells_for(manifest, runner)
    workloads = {w["name"]: w for w in manifest["workloads"]}
    report = {"schema": 1, "kind": args.command, "status": "running", "cells": cells,
              "manifest": manifest, "provenance": provenance, "samples": [], "assembly": [],
              "timing_scope": "fresh process creation through exit; log write excluded; assembly separate",
              "gpu_execution": False, "limitations": "Fresh process/session with warmed filesystem and toolkit caches, including NVRTC PCH when enabled. Compile/assembly observations; no kernel-speed inference. PTX bytes include text/symbols; cubin bytes include metadata; SASS counts and executable text sizes cover the whole module."}
    path = args.output / "measurements.json"
    write(path, report)
    try:
        samples, assembly = expected_inventory([cell["id"] for cell in cells], args.command)
        by_id = {cell["id"]: cell for cell in cells}
        latest = {}
        for name, rd, index, warmup in samples:
            cell = by_id[name]
            directory = args.output / f"round-{rd}" / name
            directory.mkdir(parents=True, exist_ok=True)
            ptx = directory / f"attempt-{index}.ptx"
            row = run(runner.compile_command(cell, workloads[cell["workload"]], args)
                      + workloads[cell["workload"]].get("compiler_options", [])
                      + ["-o", str(ptx)], ptx.with_suffix(".log"), environment, args.timeout)
            row.update(id=name, round=rd, index=index, warmup=warmup)
            report["samples"].append(row)
            write(path, report)
            require_success(row)
            text = ptx.read_text()
            if not re.search(r"^\s*\.target\s+sm_" + str(cell["architecture"]) + r"\b", text, re.M) \
                    or not re.search(r"\.entry\s+" + re.escape(cell["entry"]) + r"\s*\(", text):
                raise ValueError("PTX target or entry mismatch: " + name)
            row.update(ptx=str(ptx), ptx_sha256=sha(ptx), ptx_bytes=ptx.stat().st_size,
                       phase_ms={key: float(value) for key, value in runner.TIMER.findall(
                           Path(row["log"]).read_text())})
            latest[name] = ptx
            write(path, report)
            print(f"compile {name} round={rd} attempt={index}", flush=True)
        for name, index, warmup in assembly:
            directory = args.output / "assembly" / name
            directory.mkdir(parents=True, exist_ok=True)
            cubin = directory / f"attempt-{index}.cubin"
            row = run([args.cuda_root / "bin/ptxas", "-v",
                       f"-arch=sm_{by_id[name]['architecture']}", latest[name], "-o", cubin],
                      cubin.with_suffix(".log"), environment, args.timeout)
            row.update(id=name, index=index, warmup=warmup)
            report["assembly"].append(row)
            write(path, report)
            require_success(row)
            row.update(cubin=str(cubin), cubin_sha256=sha(cubin), cubin_bytes=cubin.stat().st_size,
                       resources=parse_resources(Path(row["log"]).read_text()))
            row["entry_resources"] = [item for item in row["resources"]
                                      if item["function"] == by_id[name]["entry"]]
            if args.command == "quality":
                cuobjdump = args.cuda_root / "bin/cuobjdump"
                row["sass"] = {"status": "unavailable"}
                if cuobjdump.is_file():
                    sass = run([cuobjdump, "--dump-sass", cubin], directory / "sass.log",
                               environment, args.timeout)
                    row["sass"] = sass
                    write(path, report)
                    require_success(sass)
                    sass["tool"] = reference(cuobjdump)
                    sass["scope"] = "whole-module"
                    sass["instruction_count"] = len(re.findall(
                        r"/\*[0-9a-fA-F]+\*/\s+\S", Path(sass["log"]).read_text()))
                row["executable_sections"] = {"status": "unavailable"}
                sections = run(["readelf", "-SW", cubin], directory / "sections.log", environment, 30)
                if sections["return_code"] == 0:
                    sizes = re.findall(r"\.text\.[^\s]+\s+PROGBITS\s+[0-9a-f]+\s+[0-9a-f]+\s+([0-9a-f]+)",
                                       Path(sections["log"]).read_text(), re.I)
                    sections["scope"] = "whole-module executable .text sections"
                    sections["text_bytes"] = sum(int(size, 16) for size in sizes) if sizes else None
                row["executable_sections"] = sections
            write(path, report)
        validate_measurements(report)
        verify_identity(provenance)
        report["status"] = "passed"
    except (OSError, ValueError, KeyError) as error:
        report.update(status="failed", error=str(error))
    write(path, report)
    return report


def checkpoint(args):
    baseline = accepted_baseline(args.baseline)
    runner, environment, provenance = configure(args, args.output)
    surface_results = load_surface_results()
    baseline_inputs = read(args.baseline).get("runtime_input_sha256", {})
    current_inputs = {name: sha(REPO / name) if (REPO / name).is_file() else None
                      for name in baseline_inputs}
    input_deltas = {name: {"before": digest, "after": current_inputs[name]}
                    for name, digest in baseline_inputs.items() if current_inputs[name] != digest}
    record = {"input_deltas": input_deltas, "schema": 1, "kind": "checkpoint", "status": "running", "provenance": provenance,
              "gates": {}, "limitations": "Additional unit/semantic/toolkit gates are required by WORKFLOW."}
    path = args.output / "checkpoint.json"
    write(path, record)
    try:
        common = ["--config", args.build_label, "--bin-dir", args.slangc.parent,
                  "--provider", args.provider, "--architecture", "80", "--jobs", str(args.jobs)]
        commands = [
            ("runtime", [sys.executable, REPO / "extras/validate-nvvm-runtime.py",
                         "--config", args.build_label, "--bin-dir", args.slangc.parent,
                         "--provider", args.provider, "--cuda-path", args.cuda_root,
                         "--architecture", "80", "--output", args.output / "runtime"]),
            ("surfaces", [sys.executable, REPO / "extras/validate-nvvm-surfaces.py",
                          "--slangc", args.slangc, "--provider", args.provider.parent,
                          "--cuda-root", args.cuda_root, "--architecture", "80",
                          "--provenance", args.output / "provenance.json",
                          "--output", args.output / "surfaces"]),
            ("frozen", [sys.executable, HERE / "run-compute-census.py", *common,
                        "--workload-ids-from", HERE / "census.slice-195.tsv",
                        "--output", args.output / "frozen"]),
            ("discovery", [sys.executable, HERE / "run-compute-discovery.py", *common,
                           "--manifest", HERE / "discovery-corpus.manifest.tsv",
                           "--output", args.output / "discovery"]),
            ("complex", [sys.executable, HERE / "run-complex-corpus.py", "--slangc", args.slangc,
                         "--provider", args.provider, "--cuda-root", args.cuda_root,
                         "--build-label", args.build_label, "--warmup", "0", "--samples", "1",
                         "--output", args.output / "complex"]),
        ]
        for name, command in commands:
            row = run(command, args.output / (name + ".log"), environment, 1800)
            record["gates"][name] = row
            write(path, record)
            if name == "surfaces":
                # The raw harness deliberately exits1 for known negatives. Prove every phase,
                # resource and physical readback before comparing their preserved failures.
                if row["timed_out"]:
                    raise ValueError("surface checkpoint timed out")
                surface_block = surface_results.validate_report(
                    args.output / "surfaces/results.json", row["return_code"])
                for key, selected in (("slangc", args.slangc), ("provider", args.provider)):
                    identity = surface_block["identity"][key]
                    if Path(identity["path"]).resolve() != selected.resolve() or identity["sha256"] != sha(selected):
                        raise ValueError("surface gate selected a different " + key)
                write(args.output / "surfaces-validated.json", surface_block)
                surface_comparison = surface_results.compare(baseline.get("surfaces"), surface_block)
                record["surface_comparison"] = surface_comparison
                write(args.output / "surface-comparison.json", surface_comparison)
                write(path, record)
                continue
            # Census exit 2 can represent established unsupported cells. Exact outcomes decide.
            if name not in ("frozen", "discovery") or row["return_code"] not in (0, 2):
                require_success(row)
            if row["timed_out"]:
                raise ValueError("checkpoint timed out: " + name)
            if name == "runtime":
                smoke = read(args.output / "runtime/results.json")
                fixture_ids = [item["fixture"] for item in smoke["results"]]
                if smoke["status"] != "passed" or len(fixture_ids) != 4 \
                        or len(set(fixture_ids)) != 4 \
                        or set(fixture_ids) != set(smoke["expected_fixtures"]) \
                        or any(item["classification"] != "correct"
                               or item["execution_counts"] != {"passed": 1, "executed": 1,
                                                              "ignored": 0, "other_summary_status": 0}
                               for item in smoke["results"]):
                    raise ValueError("runtime smoke did not execute four fixtures")
        comparison = compare(SimpleNamespace(baseline=args.baseline,
                             frozen=args.output / "frozen/results.json",
                             discovery=args.output / "discovery/results.json",
                             allow_additions=False, output=args.output))
        complex_result = read(args.output / "complex/results.json")
        expected = cells_for(runner.read_workloads(HERE / "complex-corpus.manifest.json",
                                                   runner.load_toolkit_helpers()), runner)
        if sorted(c["id"] for c in complex_result["cells"]) != sorted(c["id"] for c in expected):
            raise ValueError("complex inventory mismatch")
        if any(c["status"] != "passed" for c in complex_result["cells"]):
            raise ValueError("complex compile/assembly failure")
        verify_identity(provenance)
        comparison["surfaces"] = surface_comparison
        if surface_comparison["status"] != "passed":
            comparison["status"] = "review-required"
        write(args.output / "comparison.json", comparison)
        record["status"] = "review-required" if input_deltas else comparison["status"]
        compact = read(args.output / "outcomes.json")
        compact.update(provenance=provenance, runtime_input_sha256=current_inputs,
                       surfaces=surface_block, complex_corpus=complex_result,
                       status="corpus-" + record["status"])
        write(args.output / "outcomes.json", compact)
    except (OSError, ValueError, KeyError) as error:
        record.update(status="failed", error=str(error))
    write(path, record)
    return record


def corpus(args):
    """Replay a tier through the existing corpus runners; acceptance is read-only."""
    spec = importlib.util.spec_from_file_location("corpus_tiers", HERE / "nvvm-corpus-tiers.py")
    tiers = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(tiers)
    metadata = read(args.tier_manifest)
    inventory = tiers.build_inventory(REPO, accepted_baseline(args.baseline),
                                      read(args.focused), metadata)
    selected = tiers.select(inventory, args.tier, args.modes, args.limit)
    record = dict(kind="corpus-tier", tier=args.tier, status="listed" if args.list_only else "running",
                  scope=metadata["scope"], counts=dict(collections.Counter(row["tier"] for row in inventory)),
                  inventory_count=len(inventory), selected_count=len(selected),
                  references={name: reference(path) for name, path in (
                      ("baseline", args.baseline), ("focused", args.focused),
                      ("tier_manifest", args.tier_manifest))}, cells=selected, gates={})
    write(args.output / "inventory.json", inventory)
    path = args.output / "selection.json"
    write(path, record)
    if args.list_only:
        return dict(record, status="passed")
    _, environment, provenance = configure(args, args.output)
    identity_paths = [HERE / name for name in ("nvvm-corpus-tiers.py", "nvvm-results.py",
                      "run-compute-census.py", "run-compute-discovery.py")]
    identity_paths.extend([args.baseline, args.focused, args.tier_manifest])
    identity_paths.extend(REPO / name for cell in selected for name in cell["input_sha256"])
    provenance["artifact_sha256"].update({str(p.resolve()): sha(p) for p in identity_paths})
    write(args.output / "provenance.json", provenance)
    actual = []
    try:
        for origin in ("frozen", "discovery", "candidate"):
            group = [row for row in selected if row["origin"] == origin]
            if not group:
                continue
            selection = args.output / (origin + "-cells.json")
            write(selection, [{key: row[key] for key in ("id", "source", "source_test_ordinal", "mode")}
                              for row in group])
            modes = [mode for mode in MODES if any(row["mode"] == mode for row in group)]
            script = "run-compute-discovery.py" if origin == "discovery" else "run-compute-census.py"
            command = [sys.executable, HERE / script, "--config", args.build_label,
                       "--bin-dir", args.slangc.parent, "--provider", args.provider,
                       "--architecture", "80", "--jobs", str(args.jobs),
                       "--cells-from", selection, "--output", args.output / origin,
                       "--modes", *modes]
            if origin == "frozen":
                command += ["--workload-ids-from", HERE / "census.slice-195.tsv"]
            if args.tier != "exploratory":
                command += ["--require-all-correct"]
            gate = run(command, args.output / (origin + ".log"), environment, args.timeout)
            record["gates"][origin] = gate
            write(path, record)
            if gate["timed_out"] or gate["return_code"] not in (0, 2):
                raise ValueError("corpus runner did not complete: " + origin)
            rows = read(args.output / origin / "results.json")
            indexed = index_outcomes(rows)
            if set(indexed) != {(row["id"], row["mode"]) for row in group}:
                raise ValueError("corpus runner changed selected cell inventory: " + origin)
            actual.extend(rows)
        indexed = index_outcomes(actual)
        record["regressions"] = [{"id": row["id"], "mode": row["mode"],
                                  "actual": indexed[row["id"], row["mode"]]}
                                 for row in selected if row["tier"] == "working"
                                 and not tiers.correct(indexed[row["id"], row["mode"]])]
        record["input_changes"] = [dict(id=row["id"], mode=row["mode"], changes=row["input_changes"])
                                   for row in selected if row["input_changes"]]
        # Exploratory completion never promotes a pass or changes an accepted failure history.
        record["outcomes"] = actual
        record["status"] = ("failed" if record["regressions"] else "review-required"
                            if record["input_changes"] else "passed")
        verify_identity(provenance)
    except (OSError, ValueError, KeyError, TypeError) as error:
        record.update(status="failed", error=str(error))
    write(path, record)
    return record


def stats(values):
    quartiles = statistics.quantiles(values, n=4, method="inclusive") if len(values) > 1 else values * 3
    return {"n": len(values), "median": statistics.median(values), "q1": quartiles[0],
            "q3": quartiles[2], "minimum": min(values), "maximum": max(values)}


def summarize(measurements):
    validate_measurements(measurements)
    if measurements["status"] != "passed":
        raise ValueError("cannot summarize an unaccepted measurement run")
    cells = []
    for cell in measurements["cells"]:
        samples = [row for row in measurements["samples"]
                   if row["id"] == cell["id"] and not row["warmup"]]
        assembly = [row for row in measurements["assembly"]
                    if row["id"] == cell["id"] and not row["warmup"]]
        cells.append({"id": cell["id"],
                      "wall_ms": stats([row["elapsed_seconds"] * 1000 for row in samples]),
                      "round_wall_ms": {str(rd): stats([row["elapsed_seconds"] * 1000
                                                        for row in samples if row["round"] == rd])
                                        for rd in sorted({row["round"] for row in samples})},
                      "phase_ms": {key: stats([row["phase_ms"][key] for row in samples])
                                   for key in sorted(samples[0]["phase_ms"])},
                      "assembly_ms": stats([row["elapsed_seconds"] * 1000 for row in assembly]),
                      "ptx_bytes": samples[0]["ptx_bytes"], "cubin_bytes": assembly[0]["cubin_bytes"],
                      "resources": assembly[0].get("resources"),
                      "entry_resources": assembly[0].get("entry_resources"),
                      "sass": assembly[0].get("sass"),
                      "executable_sections": assembly[0].get("executable_sections")})
    return {"schema": 1, "kind": measurements["kind"], "status": "passed", "cells": cells,
            "compile_attempts": len(measurements["samples"]),
            "assembly_attempts": len(measurements["assembly"]),
            "provenance": measurements["provenance"], "limitations": measurements["limitations"]}


def compact_provenance(provenance):
    """Export tool identities without copying the exhaustive source/input map into each report.

    Keep old measurement records readable: their original complete map remains when they predate
    the explicit runtime inventory. New measurements retain that full map in the hashed raw record.
    This only changes exported presentation metadata, never validation or acceptance obligations.
    """
    result = {key: value for key, value in provenance.items()
              if key not in ("artifact_sha256", "runtime_artifact_sha256")}
    complete = provenance.get("artifact_sha256", {})
    runtime = provenance.get("runtime_artifact_sha256")
    result["artifact_sha256"] = dict(complete if runtime is None else runtime)
    result["full_identity_count"] = len(complete)
    result["identity_scope"] = "legacy-full-map" if runtime is None else "runtime-and-tools"
    return result


def stage_attribution(measurements):
    """Partition each validated material sample before summarizing disjoint timing scopes."""
    if measurements["kind"] != "material":
        raise ValueError("stage attribution requires material measurements")
    definitions = {
        "builtin": "Built-in module loading",
        "front_end": "Slang front end (includes semantic checking and IR generation)",
        "link": "Slang IR linking and optimization",
        "direct_host": "Target preparation/emission, including NVVM serialization",
        "vendor_verify": "Separate libNVVM verification call (none exposed for NVRTC)",
        "vendor_compile": "Vendor compile API call, including NVRTC source compilation",
        "other": "Remaining fresh-process wall time, including startup/teardown and library loading",
    }
    common = "loadBuiltinModule frontEndExecute generateOutput linkAndOptimizeIR loadDownstreamCompiler".split()
    nvvm = ("emitNVVMForEntryPoints nvvmLegalizeIR nvvmPlanEmission nvvmLoadIRBuilder "
            "nvvmEmitIR nvvmSerializeIR nvvmSerializeQuery nvvmSerializeWrite nvvmDownstreamCompile "
            "nvvmReadLibdevice nvvmCreateProgram nvvmAddModule nvvmAddLibdevice nvvmVerifyProgram "
            "nvvmCompileProgram nvvmGetPTXSize nvvmGetPTX").split()
    nvrtc = ("emitEntryPointsSourceFromIR nvrtcDownstreamCompile nvrtcCreateProgram "
             "nvrtcCompileProgram nvrtcGetPTXSize nvrtcGetPTX").split()
    children = {
        "nvvmSerializeIR": "nvvmSerializeQuery nvvmSerializeWrite",
        "nvvmEmitIR": "nvvmSerializeIR",
        "nvvmDownstreamCompile": "nvvmReadLibdevice nvvmCreateProgram nvvmAddModule nvvmAddLibdevice nvvmVerifyProgram nvvmCompileProgram nvvmGetPTXSize nvvmGetPTX",
        "emitNVVMForEntryPoints": "linkAndOptimizeIR nvvmLegalizeIR nvvmPlanEmission nvvmLoadIRBuilder nvvmEmitIR loadDownstreamCompiler nvvmDownstreamCompile",
        "nvrtcDownstreamCompile": "nvrtcCreateProgram nvrtcCompileProgram nvrtcGetPTXSize nvrtcGetPTX",
        "emitEntryPointsSourceFromIR": "linkAndOptimizeIR",
    }
    cells, rounded = [], 0
    for cell in measurements["cells"]:
        is_nvvm = cell["backend"] == "nvvm"
        if cell["backend"] not in ("nvvm", "nvrtc"):
            raise ValueError("unsupported stage-attribution backend")
        required = common + (nvvm if is_nvvm else nvrtc)
        rows = []
        for sample in (row for row in measurements["samples"] if row["id"] == cell["id"]):
            phases = sample["phase_ms"]
            entries = re.findall(r"^\[\*\]\s+(\w+)\s+(\d+)\s+([\d.]+)ms", Path(sample["log"]).read_text(), re.M)
            parsed = {name: (int(count), float(value)) for name, count, value in entries}
            if len(parsed) != len(entries) or any(name not in phases or name not in parsed
                    or parsed[name] != (2 if name == "loadBuiltinModule" else 1, phases[name])
                    or not math.isfinite(phases[name]) or phases[name] < 0 for name in required):
                raise ValueError("missing, invalid or inconsistent attribution scopes: " + sample["log"])
            if any(name in phases for name in (nvrtc if is_nvvm else nvvm)):
                raise ValueError("mixed backend attribution scopes")
            local_children = dict(children)
            local_children["generateOutput"] = ("emitNVVMForEntryPoints" if is_nvvm else
                "emitEntryPointsSourceFromIR nvrtcDownstreamCompile loadDownstreamCompiler")
            for parent, names in local_children.items():
                if parent in phases and phases[parent] - sum(phases[n] for n in names.split()) < -0.01 * (len(names.split()) + 1):
                    raise ValueError("impossible attribution containment: " + parent)
            wall = sample["elapsed_seconds"] * 1000
            if not math.isfinite(wall) or wall <= 0:
                raise ValueError("invalid attribution wall duration")
            # These three outer intervals are disjoint in the instrumented CLI path.
            if wall - sum(phases[name] for name in
                          ("loadBuiltinModule", "frontEndExecute", "generateOutput")) < -0.04:
                raise ValueError("impossible attribution root containment")
            parts = {"builtin": phases["loadBuiltinModule"], "front_end": phases["frontEndExecute"],
                     "link": phases["linkAndOptimizeIR"],
                     "direct_host": sum(phases[n] for n in ("nvvmLegalizeIR", "nvvmPlanEmission", "nvvmLoadIRBuilder", "nvvmEmitIR")) if is_nvvm else phases["emitEntryPointsSourceFromIR"] - phases["linkAndOptimizeIR"],
                     "vendor_verify": phases["nvvmVerifyProgram"] if is_nvvm else 0.0,
                     "vendor_compile": phases["nvvmCompileProgram" if is_nvvm else "nvrtcCompileProgram"]}
            parts["other"] = wall - sum(parts.values())
            if any(not math.isfinite(value) or value < -0.08 for value in parts.values()):
                raise ValueError("impossible attribution residual")
            rounded += sum(value < 0 for value in parts.values())
            parts = {name: max(0.0, value) for name, value in parts.items()}
            if not sample["warmup"]:
                rows.append({"round": sample["round"], "wall": wall, "parts": parts})
        groups = {"combined": rows, **{str(rd): [row for row in rows if row["round"] == rd]
                                      for rd in sorted({row["round"] for row in rows})}}
        cells.append({**cell, "groups": {group: {"wall_ms": stats([row["wall"] for row in values]),
            "stages": {name: {"ms": stats([row["parts"][name] for row in values]),
                               "percent_wall": stats([100 * row["parts"][name] / row["wall"] for row in values])}
                       for name in definitions}} for group, values in groups.items()}})
    return {"schema": 1, "kind": "material-stage-attribution", "status": "passed",
            "definitions": definitions, "cells": cells, "rounded_residual_count": rounded,
            "method": "Partition every sample before computing inclusive quartiles and medians; warmups excluded. Both rounds retained. Displayed phase resolution is 0.01ms; residuals within 0.08ms of zero are rounded to zero. Medians and percentages need not add up across stages.",
            "limitations": "NVVM target work includes legalization, planning, provider loading, capability checks, construction, serialization and module teardown; NVRTC target work is source emission excluding linking. Vendor compile is an opaque API duration, not pure optimization. Zero separate NVRTC verification means no separate exposed call, not absence of internal validation. No assembly or GPU time is included."}


def write_stage_attribution(result, output, plt):
    """Render independent stage medians and IQRs; do not stack marginal medians."""
    write(output / "stage-attribution.json", result)
    lines = ["# Material compilation stage attribution", "", result["method"], "", result["limitations"], "",
             "| Cell / round | Stage | Median ms (IQR) | Median % wall (IQR) |",
             "| --- | --- | ---: | ---: |"]
    for cell in result["cells"]:
        for group, values in cell["groups"].items():
            for name, stage in values["stages"].items():
                ms, pct = stage["ms"], stage["percent_wall"]
                lines.append(f"| {cell['id']} / {group} | {result['definitions'][name]} | "
                             f"{ms['median']:.2f} ({ms['q1']:.2f}–{ms['q3']:.2f}) | "
                             f"{pct['median']:.2f} ({pct['q1']:.2f}–{pct['q3']:.2f}) |")
    lines += ["", "![Independent stage medians](stage-attribution.svg)", "",
              "Full statistics and provenance: [stage-attribution.json](stage-attribution.json)."]
    (output / "stage-attribution.md").write_text("\n".join(lines) + "\n")
    entries = list(dict.fromkeys((cell["workload"], cell["entry"]) for cell in result["cells"]))
    figure, axes = plt.subplots(len(entries), 1, figsize=(12, 5 * len(entries)), squeeze=False)
    labels = ["Built-in loading", "Front end", "Link/optimize IR", "Target preparation/emission",
              "Separate vendor verify", "Vendor compile API", "Other wall time"]
    for axis, entry in zip(axes[:, 0], entries):
        selected = [cell for cell in result["cells"] if (cell["workload"], cell["entry"]) == entry]
        for index, cell in enumerate(selected):
            stages = list(cell["groups"]["combined"]["stages"].values())
            values = [stage["ms"] for stage in stages]
            axis.barh([pos + (index - 1) * 0.24 for pos in range(len(labels))],
                      [value["median"] for value in values], height=0.22,
                      xerr=[[value["median"] - value["q1"] for value in values],
                            [value["q3"] - value["median"] for value in values]], capsize=2,
                      label=f"{cell['backend'].upper()} O{cell['optimization']}")
        axis.set_yticks(range(len(labels)), labels)
        axis.invert_yaxis()
        axis.set_title(" / ".join(entry))
        axis.set_xlabel("Independent stage duration (ms), median and IQR; bars are not additive")
        axis.grid(axis="x", alpha=0.2)
        axis.legend()
    figure.tight_layout()
    for suffix in ("svg", "png"):
        figure.savefig(output / ("stage-attribution." + suffix), dpi=150)
    plt.close(figure)


def report(args):
    measurements = read(args.measurements)
    summary = summarize(measurements)
    attribution = stage_attribution(measurements) if getattr(args, "stage_attribution", False) else None
    summary["measurements"] = reference(args.measurements)
    summary["provenance"] = compact_provenance(summary["provenance"])
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ImportError as error:
        raise ValueError("report requires matplotlib; see RESULTS.md report environment") from error
    summary["plotting"] = {"matplotlib": matplotlib.__version__}
    if attribution is not None:
        attribution.update(measurements=summary["measurements"], provenance=summary["provenance"],
                           plotting=summary["plotting"])
        write_stage_attribution(attribution, args.output, plt)
        summary["stage_attribution"] = "stage-attribution.json"
    write(args.output / "summary.json", summary)
    lines = ["# NVVM " + summary["kind"] + " observations", "",
             "Revision: `" + summary["provenance"]["revision"] + "`.", "",
             "All times are milliseconds. Wall medians include fresh process/session overhead; "
             "assembly is separate. Nested compiler phase timers must not be added.", "",
             "| Cell | Wall median (IQR) | Assembly median | PTX bytes | Cubin bytes |",
             "| --- | ---: | ---: | ---: | ---: |"]
    for cell in summary["cells"]:
        wall = cell["wall_ms"]
        lines.append(f"| {cell['id']} | {wall['median']:.2f} ({wall['q1']:.2f}–{wall['q3']:.2f}) "
                     f"| {cell['assembly_ms']['median']:.2f} | {cell['ptx_bytes']} | {cell['cubin_bytes']} |")
    if summary["kind"] == "quality":
        lines = ["# NVVM quality observations", "",
                 "Revision: `" + summary["provenance"]["revision"] + "`."]
        lines += ["", "Quality latencies are single observations, not a speed benchmark.", "",
                  "| Entry | Registers | Stack bytes | Spill store/load bytes | Module SASS instructions | Executable text bytes |",
                  "| --- | ---: | ---: | ---: | ---: | ---: |"]
        for cell in summary["cells"]:
            entry_rows = cell.get("entry_resources") or []
            resource = entry_rows[0] if len(entry_rows) == 1 else {}
            def display(value):
                return "unavailable" if value is None else str(value)
            lines.append("| " + cell["id"] + " | " + display(resource.get("registers"))
                         + " | " + display(resource.get("stack_bytes")) + " | "
                         + display(resource.get("spill_store_bytes")) + "/"
                         + display(resource.get("spill_load_bytes")) + " | "
                         + display((cell.get("sass") or {}).get("instruction_count")) + " | "
                         + display((cell.get("executable_sections") or {}).get("text_bytes")) + " |")
    else:
        lines += ["", "| Cell | SemanticChecking median ms | compileInner median ms |",
                  "| --- | ---: | ---: |"]
        for cell in summary["cells"]:
            phase = cell["phase_ms"]
            lines.append(f"| {cell['id']} | {phase.get('SemanticChecking', {}).get('median', 'unavailable')} "
                         f"| {phase.get('compileInner', {}).get('median', 'unavailable')} |")
    quality = summary["kind"] == "quality"
    chart = "entry-registers" if quality else "wall-time"
    lines += ["", summary["limitations"], "", f"![Measured observations]({chart}.svg)", "",
              "Full phase, resource, SASS and executable-section observations: [summary.json](summary.json)."]
    (args.output / "summary.md").write_text("\n".join(lines) + "\n")
    figure, axis = plt.subplots(figsize=(12, max(4, len(summary["cells"]) * 0.32)))
    labels = [cell["id"] for cell in summary["cells"]]
    if quality:
        # Compare like optimization levels; retain O0 observations in the full table and JSON.
        names = list(dict.fromkeys(re.sub(r"-(nvrtc|nvvm)-o[03]$", "", cell["id"])
                                   for cell in summary["cells"]))
        by_id = {cell["id"]: cell for cell in summary["cells"]}
        for backend, offset, color in (("nvrtc", -0.18, "#317bba"), ("nvvm", 0.18, "#d78225")):
            values = []
            for name in names:
                entries = by_id[name + "-" + backend + "-o3"].get("entry_resources") or []
                values.append(entries[0].get("registers") if len(entries) == 1 else None)
            positions = [index + offset for index in range(len(names))]
            axis.barh(positions, [value if value is not None else 0 for value in values],
                      height=0.34, color=color, label=backend.upper() + " O3")
            for y, value in zip(positions, values):
                axis.text(value or 0, y, " unavailable" if value is None else f" {value}",
                          va="center", fontsize=8)
        axis.set_yticks(range(len(names)), names)
        figure.set_size_inches(12, max(4, len(names) * 0.6))
        axis.legend()
        axis.set_xlabel("Registers per thread, named kernel entry (ptxas)")
        axis.set_title("Entry register allocation; no GPU speed inference")
    else:
        values = [cell["wall_ms"]["median"] for cell in summary["cells"]]
        errors = [[cell["wall_ms"]["median"] - cell["wall_ms"]["q1"] for cell in summary["cells"]],
                  [cell["wall_ms"]["q3"] - cell["wall_ms"]["median"] for cell in summary["cells"]]]
        axis.barh(labels, values, xerr=errors, color="#317bba", capsize=3)
        axis.set_xlabel("Fresh process wall time (ms); median and interquartile range")
        axis.set_title("Material compilation; warmed filesystem/toolkit caches")
    axis.invert_yaxis()
    axis.grid(axis="x", alpha=0.2)
    figure.tight_layout()
    for suffix in ("svg", "png"):
        figure.savefig(args.output / (chart + "." + suffix), dpi=150)
    plt.close(figure)
    return summary


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    sub = parser.add_subparsers(dest="command", required=True)
    for name in ("compare", "checkpoint", "material", "quality", "report"):
        command = sub.add_parser(name)
        command.add_argument("--output", type=Path, required=True, help="new directory; never overwritten")
        if name in ("compare", "checkpoint"):
            command.add_argument("--baseline", type=Path, required=True)
        if name == "compare":
            command.add_argument("--frozen", type=Path, required=True)
            command.add_argument("--discovery", type=Path, required=True)
            command.add_argument("--allow-additions", action="store_true")
        if name in ("checkpoint", "material", "quality"):
            command.add_argument("--slangc", type=Path, required=True)
            command.add_argument("--provider", type=Path, required=True)
            command.add_argument("--cuda-root", type=Path, required=True)
            command.add_argument("--build-label", choices=["RelWithDebInfo", "Release"], required=True)
            command.add_argument("--timeout", type=int, default=180)
        if name in ("material", "quality"):
            command.add_argument("--manifest", type=Path, default=HERE / (
                "complex-corpus.manifest.json" if name == "material" else "quality-corpus.manifest.json"))
        if name == "quality":
            command.add_argument("--correctness", type=Path, required=True,
                                 help="reviewed accepted-full checkpoint with matching binaries/inputs")
        if name == "checkpoint":
            command.add_argument("--jobs", type=int, choices=range(1, 5), default=4)
        if name == "report":
            command.add_argument("--measurements", type=Path, required=True)
            command.add_argument("--stage-attribution", action="store_true",
                                 help="report disjoint material stages; requires instrumented phase logs")
    command = sub.add_parser("corpus", help="select existing runtime corpus tiers")
    command.add_argument("--output", type=Path, required=True)
    command.add_argument("--tier", choices=["smoke", "working", "exploratory"], required=True)
    command.add_argument("--list-only", action="store_true", help="write inventory without invoking tools")
    command.add_argument("--modes", nargs="+", choices=MODES)
    command.add_argument("--limit", type=int, help="maximum cells in an exploratory batch")
    command.add_argument("--tier-manifest", type=Path, default=HERE / "corpus-tiers.json")
    command.add_argument("--baseline", type=Path, default=HERE / "accepted-baseline.json")
    command.add_argument("--focused", type=Path, default=HERE / "focused-evidence.json")
    command.add_argument("--slangc", type=Path, default=REPO / "build/RelWithDebInfo/bin/slangc")
    command.add_argument("--provider", type=Path, default=REPO / "build/RelWithDebInfo/bin/libslang-llvm-nvvm.so")
    command.add_argument("--cuda-root", type=Path, default=Path("/usr/local/cuda-12.9"))
    command.add_argument("--build-label", choices=["RelWithDebInfo", "Release"], default="RelWithDebInfo")
    command.add_argument("--jobs", type=int, choices=range(1, 9), default=4)
    command.add_argument("--timeout", type=int, default=1800)
    args = parser.parse_args()
    args.output = args.output.resolve()
    try:
        if getattr(args, "timeout", 1) <= 0:
            raise ValueError("timeout must be positive")
        args.output.mkdir(parents=True, exist_ok=False)
        result = {"compare": compare, "checkpoint": checkpoint, "material": measure,
                  "quality": measure, "report": report, "corpus": corpus}[args.command](args)
        print(result["status"] + ": " + str(args.output))
        return 0 if result["status"] == "passed" else 1
    except (OSError, ValueError, KeyError, TypeError, subprocess.SubprocessError) as error:
        print(str(error), file=sys.stderr)
        return 2


if __name__ == "__main__":
    sys.exit(main())
