#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Capture final specialized PTX from original corpus dispatches, retaining every outcome.

This diagnostic run collects no performance samples. Each case retains its original bindings,
ordinary oracle, and the accepted runtime g0/backend/optimization options. Compilation dumping
writes the final PTX artifact returned by getEntryPointCode; CUDA RHI loads that blob unchanged.
Only a unique PTX module with one entry and a passing original oracle qualifies for code analysis.
"""

import argparse
import collections
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import sys

REPO = Path(__file__).resolve().parents[1]
HERE = REPO / "issue-nvvm-backend"
MODES = ("nvrtc-o3", "nvvm-o0", "nvvm-o3")


def load(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def save(path, value):
    Path(path).write_text(json.dumps(value, indent=2, allow_nan=False) + "\n")


def select_manifest(manifest, current, case_ids, source_hash):
    """Require the accepted full selection and unchanged source/command identities before filtering."""
    if manifest.get("kind") != "corpus-dispatch-performance" or manifest.get("selection") != "full-frozen-and-discovery" \
            or manifest.get("debug_info") != "g0":
        raise ValueError("runtime manifest is not the full accepted g0 selection")
    selected = manifest["selected"]
    if len({row["id"] for row in selected}) != len(selected):
        raise ValueError("duplicate runtime case")
    if [row["id"] for row in selected] != [row["id"] for row in current]:
        raise ValueError("runtime manifest selection differs from current corpora")
    for old, new in zip(selected, current):
        if any(old.get(key) != value for key, value in new.items()):
            raise ValueError("runtime command identity changed: " + old["id"])
        if source_hash(old["source"]) != old["source_sha256"]:
            raise ValueError("runtime source changed: " + old["id"])
    if case_ids and (len(set(case_ids)) != len(case_ids) or set(case_ids) - {row["id"] for row in selected}):
        raise ValueError("duplicate or unknown requested case")
    return [row for row in selected if not case_ids or row["id"] in case_ids]


def inspect_ptx(artifacts):
    """Reject missing or ambiguous dumps; ordinal filenames never identify the loaded module."""
    paths = [Path(row["path"]) for row in artifacts if Path(row["path"]).suffix == ".ptx"]
    if len(paths) != 1:
        return dict(status="missing-ptx" if not paths else "ambiguous-ptx", ptx_count=len(paths))
    path = paths[0]
    text = re.sub(r"/\*.*?\*/|//[^\n]*", "", path.read_text(), flags=re.S)
    entries = re.findall(r"\.entry\s+([\w$]+)\s*\(", text)
    targets = re.findall(r"^\s*\.target\s+sm_(\d+)\b", text, re.M)
    if len(entries) != 1 or targets != ["80"]:
        return dict(status="unsupported-module-inventory", entries=entries, targets=targets)
    return dict(status="captured", ptx=str(path), entry=entries[0], target="sm_80")


def qualification(oracle, accepted, capture):
    """Keep known failures distinct from fresh oracle regressions and diagnostic capture failures."""
    if oracle not in ("passed", "FAILED"):
        return "unverified-oracle"
    if accepted != "correct":
        return "known-oracle-gap" if oracle == "FAILED" else "oracle-transition-review-required"
    if oracle == "FAILED":
        return "oracle-regression-review-required"
    return "qualified" if capture["status"] == "captured" else "capture-excluded"


def prepare(census, output, selected, mode):
    root = output / "mirrors" / mode
    census.prepare_mode(REPO / "tests", output, root, selected, mode)
    cells = []
    for row in selected:
        generated = root / census._generated_relative_path(row)
        key = hashlib.sha256(row["id"].encode()).hexdigest()[:20]
        dump = output / "dumps" / mode / key
        dump.mkdir(parents=True, exist_ok=False)
        directive, source = generated.read_text().split("\n", 1)
        backend, optimization = mode.split("-o")
        # Legacy overrides must be last: render-test extracts uppercase -Xslang first.
        directive += (f" -g0 -compile-arg -g0 -compile-arg -O{optimization}"
                      f" -compile-arg -emit-cuda-via-{backend} -compile-arg -dump-intermediates"
                      f" -compile-arg -dump-intermediate-prefix -compile-arg {dump}/artifact-")
        generated.write_text(directive + "\n" + source)
        cells.append(dict(id=row["id"], corpus=row["corpus"], source=row["source"], mode=mode,
                          generated=str(generated), dump=str(dump),
                          accepted_outcome=row["accepted_outcomes"][mode]))
    return root, cells


def collect(args):
    nr = load("capture_results", HERE / "nvvm-results.py")
    runtime = load("capture_runtime", REPO / "extras/measure-nvvm-corpus-runtime.py")
    census = load("capture_census", HERE / "run-compute-census.py")
    discovery = load("capture_discovery", HERE / "run-compute-discovery.py")
    prior = nr.read(args.runtime_manifest)
    selected = select_manifest(prior, runtime.select(census, discovery, None), args.case,
                               lambda source: nr.sha(REPO / "tests" / source))
    old_provenance = prior["provenance"]
    if nr.sha(Path(old_provenance["path"])) != old_provenance["sha256"]:
        raise ValueError("runtime provenance identity changed")
    old = nr.read(old_provenance["path"])
    # Include tests, inputs and dependencies, not only selected source entry files.
    test_prefix = str(REPO / "tests") + "/"
    original_inputs = {path: digest for path, digest in old["artifact_sha256"].items()
                       if path.startswith(test_prefix)}
    if not original_inputs:
        raise ValueError("runtime provenance lacks original test inputs")
    nr.verify_identity(dict(artifact_sha256=original_inputs))
    args.output = args.output.resolve()
    args.output.relative_to(REPO)
    if any(char.isspace() or char in "\"'" for char in str(args.output)):
        raise ValueError("test directives require an output path without whitespace or quotes")
    args.output.mkdir(parents=True, exist_ok=False)
    args.command = "corpus-code-capture"
    _, environment, provenance = nr.configure(args, args.output)
    for path, digest in old["runtime_artifact_sha256"].items():
        if provenance["runtime_artifact_sha256"].get(path) != digest:
            raise ValueError("runtime binary/tool identity differs: " + path)
    for path in (Path(__file__).resolve(), args.runtime_manifest.resolve()):
        provenance["artifact_sha256"][str(path)] = nr.sha(path)
    save(args.output / "provenance.json", provenance)
    manifest = dict(schema=1, kind="corpus-code-capture", selection="subset" if args.case else "full-frozen-and-discovery",
                    runtime_manifest=nr.reference(args.runtime_manifest), selected=selected,
                    provenance=nr.reference(args.output / "provenance.json"), modes=MODES,
                    debug_info="g0", batch_size=args.batch_size, gpu_execution=True,
                    performance_measurement=False,
                    capture_contract="Unique final getEntryPointCode PTX artifact, one entry, original oracle; no ordinal selection.")
    groups = [prepare(census, args.output, selected, mode) for mode in MODES]
    manifest["cells"] = [cell for _, cells in groups for cell in cells]
    manifest["mirror_input_sha256"] = {str(path): nr.sha(path) for root, _ in groups
                                       for path in sorted(root.rglob("*")) if path.is_file()}
    save(args.output / "manifest.json", manifest)
    report = dict(schema=1, status="running", manifest=nr.reference(args.output / "manifest.json"), batches=[], cells=[])
    save(args.output / "results.json", report)
    try:
        for root, cells in groups:
            for start in range(0, len(cells), args.batch_size):
                batch = cells[start:start + args.batch_size]
                paths = [str(Path(row["generated"]).relative_to(REPO)) for row in batch]
                names = [path + " (cuda)" for path in paths]
                command = [args.slangc.parent / "slang-test", "-test-dir", root, "-use-test-server",
                           "-server-count", "1", "-disable-retries", "-explicit-test-order", "-v", "info", *paths]
                process = nr.run(command, args.output / f"batch-{len(report['batches']):04d}.log", environment, 1800)
                process.update(names=names, log_sha256=nr.sha(Path(process["log"])))
                output = Path(process["log"]).read_text(errors="replace")
                try:
                    statuses = runtime.parse_batch(output, names, process, census)
                    process["inventory_error"] = None
                except ValueError as error:
                    statuses = {}
                    process["inventory_error"] = str(error)
                report["batches"].append(process)
                for cell, name in zip(batch, names):
                    artifacts = [dict(nr.reference(path), bytes=path.stat().st_size)
                                 for path in sorted(Path(cell["dump"]).rglob("*")) if path.is_file()]
                    capture = inspect_ptx(artifacts)
                    oracle = statuses.get(name, "unverified")
                    result = dict(cell, oracle=oracle, capture=capture, artifacts=artifacts,
                                  log=process["log"], status=qualification(oracle, cell["accepted_outcome"]["classification"], capture))
                    result["capture_status"] = capture["status"]
                    if capture["status"] == "captured":
                        result["ptx"] = nr.reference(Path(capture["ptx"]))
                        result["entry"] = capture["entry"]
                    actual = Path(cell["generated"] + ".actual.txt")
                    if actual.is_file():
                        result["actual_output"] = nr.reference(actual)
                    if oracle == "passed" and not actual.is_file():
                        result["status"] = "missing-oracle-output"
                    report["cells"].append(result)
                save(args.output / "results.json", report)
                print(f"captured {len(report['cells'])}/{len(manifest['cells'])}", flush=True)
                if process["inventory_error"]:
                    raise ValueError("batch inventory failed: " + process["inventory_error"])
        if [(row["id"], row["mode"]) for row in report["cells"]] != [(row["id"], row["mode"]) for row in manifest["cells"]]:
            raise ValueError("capture inventory differs from manifest")
        nr.verify_identity(provenance)
        nr.verify_identity(dict(artifact_sha256=manifest["mirror_input_sha256"]))
        report["status"] = "review-required" if any("review-required" in row["status"] for row in report["cells"]) else "completed"
    except (ValueError, OSError) as error:
        report.update(status="failed", error=str(error))
    report["counts"] = dict(collections.Counter(row["status"] for row in report["cells"]))
    save(args.output / "results.json", report)
    if report["status"] != "completed":
        raise ValueError("capture did not complete without oracle transitions: " + report["status"])


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--runtime-manifest", type=Path, required=True)
    parser.add_argument("--slangc", type=Path, default=REPO / "build/RelWithDebInfo/bin/slangc")
    parser.add_argument("--provider", type=Path, default=REPO / "build/RelWithDebInfo/bin/libslang-llvm-nvvm.so")
    parser.add_argument("--cuda-root", type=Path, default=Path("/usr/local/cuda-12.9"))
    parser.add_argument("--build-label", default="RelWithDebInfo")
    parser.add_argument("--batch-size", type=int, default=32)
    parser.add_argument("--case", action="append")
    args = parser.parse_args()
    if not 1 <= args.batch_size <= 128:
        parser.error("batch size must be in 1..128")
    try:
        collect(args)
    except (ValueError, OSError, KeyError) as error:
        print("error: " + str(error), file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
