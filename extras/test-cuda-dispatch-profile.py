#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""Qualify render-test CUDA dispatch replay using original COMPARE_COMPUTE oracles.

Run from the repository root with an unused --output directory. These are harness
contract tests, not performance measurements. Every subprocess/log/sidecar is retained.
"""

import argparse
import importlib.util
import json
import math
import os
from pathlib import Path
import subprocess


ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--runner", type=Path, default=ROOT / "build/RelWithDebInfo/bin/slang-test")
    args = parser.parse_args()
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=False)
    spec = importlib.util.spec_from_file_location(
        "census", ROOT / "issue-nvvm-backend/run-compute-census.py"
    )
    census = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(census)
    fixtures = [
        "tests/cuda/dispatch-profile-replay.slang",
        "tests/cuda/dispatch-profile-texture-replay.slang",
        "tests/language-feature/dynamic-dispatch/buffer-address-ref.slang",
        "tests/cuda/nvvm-aggregate-param-resource-snapshot.slang",
    ]
    results = []

    def run(name, source_path, mode="nvrtc-o3", negative=None):
        directory = output / name
        directory.mkdir()
        source = (ROOT / source_path).read_text()
        directive = next(
            item for item in census.enumerate_test_directives(source)
            if census.is_active_compare_directive(item) and "-cuda" in item["arguments"]
        )
        workload = {**directive, "capability": "cuda_sm_8_0"}
        text = census._directive_for_mode(workload, mode).rstrip()
        backend, optimization = mode.split("-o")
        text += (f" -g0 -compile-arg -g0 -compile-arg -O{optimization}"
                 f" -compile-arg -emit-cuda-via-{backend}")
        report = directory / "timing.jsonl"
        options = f" -cuda-dispatch-profile {report} -cuda-dispatch-warmups 1 -cuda-dispatch-samples 2"
        if negative == "bad-count":
            options += " -cuda-dispatch-samples 0"
        elif negative == "noninteger-count":
            options += " -cuda-dispatch-warmups 2x"
        elif negative == "count-without-profile":
            options = " -cuda-dispatch-samples 2"
        elif negative == "unwritable-report":
            options = f" -cuda-dispatch-profile {directory / 'missing/timing.jsonl'}"
        elif negative == "existing-report":
            report.write_text("preserve existing evidence\n")
        body = census.source_without_test_directives(source)
        if negative == "no-output":
            body = body.replace("out ubuffer", "ubuffer")
        elif negative == "oracle-mismatch":
            body = body.replace("// CHECK-NEXT: 17", "// CHECK-NEXT: 18")
        shader = directory / "test.slang"
        shader.write_text(text + options + "\n" + body)
        command = [
            str(args.runner.resolve()), "-test-dir", str(directory), "-disable-retries",
            "-v", "info", str(shader.relative_to(ROOT)),
        ]
        process = subprocess.run(
            command, cwd=ROOT, env=os.environ.copy(), text=True,
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=120,
        )
        (directory / "test.log").write_text(process.stdout)
        result = {"name": name, "command": command, "return_code": process.returncode}
        results.append(result)
        (output / "results.json").write_text(json.dumps(results, indent=2) + "\n")
        if negative:
            assert process.returncode != 0, (name, "expected failure")
            assert "failed test:" in process.stdout.lower(), (name, "missing executed failure")
            if negative == "existing-report":
                assert report.read_text() == "preserve existing evidence\n"
            if negative == "oracle-mismatch":
                rows = [json.loads(line) for line in report.read_text().splitlines()]
                assert rows[-1]["status"] == "completed"
            elif negative == "no-output":
                rows = [json.loads(line) for line in report.read_text().splitlines()]
                assert rows[-1]["status"] == "failed"
        else:
            assert process.returncode == 0, (name, process.stdout)
            assert census.execution_counts(process.stdout) == {
                "passed": 1, "executed": 1, "ignored": 0, "other_summary_status": 0,
            }
            rows = [json.loads(line) for line in report.read_text().splitlines()]
            assert rows[0]["schema_version"] == 1
            resources = next(row for row in rows if row["record"] == "resources")
            if "dispatch-profile-texture-replay" in source_path:
                # Array: 2*(4*4+2*2)*4, volume: (4**3+2**3)*4,
                # cube: 6*(4*4+2*2)*4, output: 3*4 bytes.
                assert resources == {
                    "record": "resources", "allocations": 5, "snapshots": 19,
                    "reset_bytes": 940, "outputs": 1,
                }
            elif "dispatch-profile-replay" in source_path:
                # Includes the implicit four-byte append counter and the texture's
                # default complete mip chain: (4*4+2*2+1)*4 bytes.
                assert resources == {
                    "record": "resources", "allocations": 8, "snapshots": 10,
                    "reset_bytes": 140, "outputs": 2,
                }
            launches = [row for row in rows if row["record"] == "launch"]
            assert [(row["phase"], row["index"]) for row in launches] == [
                ("reference", 0), ("warmup", 0), ("sample", 0), ("sample", 1),
            ]
            assert all(row["output_equal"] and math.isfinite(row["device_ms"])
                       and row["device_ms"] >= 0 and row["start_callbacks"] == 1
                       and row["end_callbacks"] == 1 for row in launches)
            assert rows[-1] == {
                "record": "summary", "status": "completed", "launches": 4,
                "warmups": 1, "samples": 2,
            }
        result["contract_passed"] = True
        (output / "results.json").write_text(json.dumps(results, indent=2) + "\n")
        print(f"PASS {name}", flush=True)

    for fixture in fixtures:
        # Layered surface writes are an existing NVVM feature gap. This fixture tests
        # the shared replay harness with NVRTC; the basic 2D reset runs in all modes.
        modes = ("nvrtc-o3",) if "texture-replay" in fixture else census.MODES
        for mode in modes:
            run(Path(fixture).stem + "-" + mode, fixture, mode)
    for negative in (
        "bad-count", "noninteger-count", "count-without-profile", "unwritable-report",
        "existing-report", "no-output", "oracle-mismatch",
    ):
        run(negative, fixtures[0], negative=negative)
    print(f"Passed {len(results)} CUDA dispatch profiling contracts.")


if __name__ == "__main__":
    main()
