#!/usr/bin/env python3

"""Measure the selected direct-NVVM compute MVP workload gates.

This records standalone compiler wall time and PTX size for NVRTC O3, direct NVVM O0, and direct
NVVM O3. Direct O3 is additionally compiled and assembled for SM70, SM80, and SM90. The census
harness timings are copied alongside these measurements as end-to-end compile/load/execute/compare
times; they are deliberately not labeled as kernel-only runtime.
"""

from __future__ import annotations

import argparse
import csv
import json
import os
from pathlib import Path
import re
import statistics
import subprocess
import time


WORKLOADS = [
    {
        "name": "resource-aggregate-helper",
        "source": "tests/compute/dynamic-dispatch-bindless-texture.slang",
        "census_id": "compute/dynamic-dispatch-bindless-texture.slang#cuda-1",
    },
    {
        "name": "parameter-block-layout",
        "source": "tests/compute/parameter-block.slang",
        "census_id": "compute/parameter-block.slang#cuda-1",
    },
    {
        "name": "shared-control-barriers",
        "source": (
            "tests/language-feature/execution-model/"
            "groupshared-multi-barrier-functional.slang"
        ),
        "census_id": (
            "language-feature/execution-model/"
            "groupshared-multi-barrier-functional.slang#cuda-1"
        ),
    },
]


COMPILE_CONFIGURATIONS = [
    {
        "name": "nvrtc-o3-native",
        "nvvm": False,
        "optimization": 3,
        "sm": 70,
    },
    {"name": "nvvm-o0-sm70", "nvvm": True, "optimization": 0, "sm": 70},
    {"name": "nvvm-o3-sm70", "nvvm": True, "optimization": 3, "sm": 70},
    {"name": "nvvm-o3-sm80", "nvvm": True, "optimization": 3, "sm": 80},
    {"name": "nvvm-o3-sm90", "nvvm": True, "optimization": 3, "sm": 90},
]


def _run(command: list[str], cwd: Path, environment: dict[str, str]) -> tuple[int, float, str]:
    started = time.perf_counter()
    completed = subprocess.run(
        command,
        cwd=cwd,
        env=environment,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        encoding="utf-8",
        errors="replace",
        check=False,
    )
    elapsed_ms = (time.perf_counter() - started) * 1000.0
    return completed.returncode, elapsed_ms, completed.stdout


def _load_census_timings(census_results: Path) -> dict[tuple[str, str], dict[str, object]]:
    result: dict[tuple[str, str], dict[str, object]] = {}
    if not census_results.is_file():
        return result
    with census_results.open("r", encoding="utf-8", newline="") as stream:
        for row in csv.DictReader(stream, delimiter="\t"):
            result[(row["id"], row["mode"])] = row
    return result


def _get_ptx_target(ptx_path: Path) -> int:
    match = re.search(r"^\.target\s+sm_(\d+)", ptx_path.read_text(encoding="utf-8"), re.MULTILINE)
    if not match:
        raise SystemExit(f"unable to find PTX target in {ptx_path}")
    return int(match.group(1))


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parents[1])
    parser.add_argument("--output", type=Path, default=Path("build/nvvm-census/mvp-metrics"))
    parser.add_argument(
        "--census-results",
        type=Path,
        default=Path("build/nvvm-census/results.tsv"),
        help="census result rows supplying end-to-end timings",
    )
    parser.add_argument("--repetitions", type=int, default=3)
    return parser.parse_args()


def main() -> int:
    args = parse_arguments()
    repo_root = args.repo.resolve()
    output_root = (
        (repo_root / args.output).resolve() if not args.output.is_absolute() else args.output
    )
    output_root.mkdir(parents=True, exist_ok=True)
    slangc = repo_root / "build/Release/bin/slangc.exe"
    provider = repo_root / "build/nvvm-builder-deps/slang-llvm-nvvm-build/Release"
    environment = os.environ.copy()
    environment["SLANG_NVVM_BUILDER_PATH"] = str(provider)
    census_results = (
        (repo_root / args.census_results).resolve()
        if not args.census_results.is_absolute()
        else args.census_results
    )
    census_timings = _load_census_timings(census_results)
    measurements: list[dict[str, object]] = []

    for workload in WORKLOADS:
        for configuration in COMPILE_CONFIGURATIONS:
            stem = f"{workload['name']}-{configuration['name']}"
            ptx_path = output_root / f"{stem}.ptx"
            diagnostic_path = output_root / f"{stem}.log"
            command = [
                str(slangc),
                str(repo_root / str(workload["source"])),
                "-target",
                "ptx",
                "-entry",
                "computeMain",
                "-stage",
                "compute",
                "-capability",
                f"cuda_sm_{int(configuration['sm']) // 10}_{int(configuration['sm']) % 10}",
                f"-O{configuration['optimization']}",
                "-o",
                str(ptx_path),
            ]
            command.append(
                "-emit-cuda-via-nvvm" if configuration["nvvm"] else "-emit-cuda-via-nvrtc"
            )
            elapsed_samples: list[float] = []
            diagnostics = ""
            return_code = 0
            for _ in range(args.repetitions):
                return_code, elapsed_ms, diagnostics = _run(command, repo_root, environment)
                elapsed_samples.append(elapsed_ms)
                if return_code:
                    break
            diagnostic_path.write_text(diagnostics, encoding="utf-8", newline="\n")
            if return_code:
                raise SystemExit(
                    f"{stem} compilation failed with {return_code}; see {diagnostic_path}"
                )

            ptx_target = _get_ptx_target(ptx_path)
            cubin_path = output_root / f"{stem}.cubin"
            ptxas_command = [
                "ptxas.exe",
                f"-arch=sm_{ptx_target}",
                str(ptx_path),
                "-o",
                str(cubin_path),
            ]
            ptxas_code, ptxas_ms, ptxas_output = _run(ptxas_command, repo_root, environment)
            (output_root / f"{stem}.ptxas.log").write_text(
                ptxas_output,
                encoding="utf-8",
                newline="\n",
            )
            if ptxas_code:
                raise SystemExit(f"{stem} ptxas failed with {ptxas_code}")

            census_mode = "nvvm-o3" if configuration["nvvm"] else "nvrtc-o3"
            if configuration["nvvm"] and configuration["optimization"] == 0:
                census_mode = "nvvm-o0"
            census = census_timings.get((str(workload["census_id"]), census_mode), {})
            measurements.append(
                {
                    "workload": workload["name"],
                    "source": workload["source"],
                    "configuration": configuration["name"],
                    "compile_median_ms": round(statistics.median(elapsed_samples), 3),
                    "compile_min_ms": round(min(elapsed_samples), 3),
                    "compile_max_ms": round(max(elapsed_samples), 3),
                    "repetitions": len(elapsed_samples),
                    "ptx_target": f"sm_{ptx_target}",
                    "ptx_bytes": ptx_path.stat().st_size,
                    "ptxas_ms": round(ptxas_ms, 3),
                    "cubin_bytes": cubin_path.stat().st_size,
                    "census_end_to_end_ms": int(census["elapsed_ms"]) if census else None,
                    "census_classification": census.get("classification", "not measured"),
                }
            )
            print(
                f"{stem}: median {statistics.median(elapsed_samples):.1f} ms, "
                f"PTX {ptx_path.stat().st_size} bytes, cubin {cubin_path.stat().st_size} bytes",
                flush=True,
            )

    (output_root / "metrics.json").write_text(
        json.dumps(measurements, indent=2) + "\n",
        encoding="utf-8",
        newline="\n",
    )
    with (output_root / "metrics.tsv").open("w", encoding="utf-8", newline="") as stream:
        writer = csv.DictWriter(
            stream,
            fieldnames=list(measurements[0]),
            delimiter="\t",
            lineterminator="\n",
        )
        writer.writeheader()
        writer.writerows(measurements)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
