#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""Qualify the unchanged tiled-brass eval entry with two synthetic live CUDA textures.

This fixed graph contract covers finite front-facing reflection at texel centers, not original
assets, LUT reads, sample_buffer, arbitrary material graphs, or GPU performance. Run on native
Linux with the selected CUDA headers. Each invocation requires a new artifact directory.
"""

import argparse
from datetime import datetime, timezone
import importlib.util
import json
import math
import os
from pathlib import Path
import platform
import re
import shutil
import struct
import subprocess
import sys

REPO = Path(__file__).resolve().parents[1]
CONTRACT = "tiled-brass-eval-synthetic-textures-v1"
ACTIVE_COUNT, OUTPUT_COUNT = 65, 128
INPUT = struct.Struct("<8fI4x")
OUTPUT = struct.Struct("<4f")
SENTINEL = b"\xa5" * OUTPUT.size
# Frozen before GPU comparison. Independent source-style float32/FMA CPU arithmetic over these
# inputs differed by <8.5e-7 absolute and <5.6e-7 relative. This leaves >400x observed arithmetic
# margin for CUDA normalization/transcendentals, while missing-lobe/texel/roughness perturbations
# exceed the comparison budget by >349x. This empirical finite-input envelope is not a proof
# about arbitrary inputs or a promise of correctly rounded CUDA transcendental instructions.
ABS_TOL, REL_TOL = 1e-5, 2e-4
COLORS = ((.02, .35, .8, 1), (.8, .15, .04, 1), (.25, .6, .1, 1), (.7, .4, .9, 1))
ROUGHNESS = (.25, .4, .6, .8)
DIRECTIONS = (((0, 0, 1), (0, 0, 1)),
              ((.3, .4, 1), (-.2, .1, 1)),
              ((-.5, .2, 1), (.4, -.3, 1)))
# MaterialX analytic directional-albedo fit, expressed as coefficients of
# [1, x, y, xy, x², y², x²y, xy², x²y²]. This is the graph's selected policy.
ALBEDO_COEFFICIENTS = (
    (.1003, .9345, 1, 1), (-.6303, -2.323, -1.765, .2281),
    (9.748, 2.229, 8.263, 15.94), (-2.038, -3.748, 11.53, -55.83),
    (29.34, 1.424, 28.96, 13.08), (-8.245, -.7684, -7.507, 41.26),
    (-26.44, 1.436, -36.11, 54.9), (19.99, .2913, 15.86, 300.2),
    (-5.448, .6286, 33.37, -285.1),
)


def load_tool(name, relative):
    spec = importlib.util.spec_from_file_location(name, REPO / relative)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def f32(value):
    """Round uploaded values once; the independent equations then use double precision."""
    return struct.unpack("<f", struct.pack("<f", value))[0]


def normalize(vector):
    length = math.sqrt(sum(value * value for value in vector))
    return tuple(value / length for value in vector)


def fresnel(index, cosine):
    """Evaluate unpolarized nonabsorbing Fresnel using Snell's law and s/p amplitudes."""
    transmitted = math.sqrt(1 - (1 - cosine * cosine) / (index * index))
    s = (cosine - index * transmitted) / (cosine + index * transmitted)
    p = (index * cosine - transmitted) / (index * cosine + transmitted)
    return (s * s + p * p) / 2


def albedo(cosine, roughness):
    """Evaluate the selected rational polynomial without reproducing shader aggregate logic."""
    x, y = cosine, roughness
    terms = (1, x, y, x*y, x*x, y*y, x*x*y, x*y*y, x*x*y*y)
    parts = [sum(row[column] * term for row, term in zip(ALBEDO_COEFFICIENTS, terms))
             for column in range(4)]
    return sum(min(1, max(0, parts[i] / parts[i + 2])) for i in (0, 1))


def linear_color(color):
    return tuple(value / 12.92 if value <= .04045 else ((value + .055) / 1.055) ** 2.4
                 for value in color[:3])


def reference(color, roughness, incoming, outgoing, omit=None):
    """Reduce this fixed two-lobe graph to isotropic GGX, Fresnel and analytic compensation.

    Both lobes share alpha=r² and their PDF, so normalized selection weights cancel. The color
    texture attenuates the conductor under the dielectric coat. The artistic IOR with reflectivity
    .99 and edge color zero has zero extinction in exact arithmetic; its large real index still
    gives a .99 normal reflectance. Source float cancellation in extinction is inside the frozen
    finite-input budget. This oracle neither compiles Slang nor consumes NVRTC output.
    """
    i, o = normalize(incoming), normalize(outgoing)
    h = normalize(tuple(a + b for a, b in zip(i, o)))
    c = i[2]
    a2 = roughness ** 4
    distribution = a2 / (math.pi * (h[2] * h[2] * (a2 - 1) + 1) ** 2)
    def smith_lambda(v):
        return (math.sqrt(1 + a2 * (1 - v[2] * v[2]) / (v[2] * v[2])) - 1) / 2
    visibility = 1 / (1 + smith_lambda(i) + smith_lambda(o))
    s2 = (1 + math.hypot(i[0], i[1])) ** 2
    k = (1 - a2) * s2 / (s2 + a2 * c * c)
    t = math.sqrt(a2 * (i[0] * i[0] + i[1] * i[1]) + c * c)
    pdf = distribution / (2 * (k * c + t))
    energy = albedo(c, roughness)
    compensation = (1 - energy) / max(energy, 1e-6)
    micro_cosine = sum(a * b for a, b in zip(h, i))
    dielectric = fresnel(1.5, micro_cosine)
    metal_index = (1 + math.sqrt(.99)) / (1 - math.sqrt(.99))
    conductor = fresnel(metal_index, micro_cosine)
    coat = dielectric * (1 + dielectric * compensation) if omit != "coat" else 0
    base = conductor * (1 + conductor * compensation) if omit != "base" else 0
    macro_transmission = 1 - fresnel(1.5, c)
    scale = distribution * visibility / (4 * c)
    return tuple(scale * (coat + component * macro_transmission * base)
                 for component in linear_color(color)) + (pdf,)


def cases():
    """Use twelve texel/direction pairs and a wrapped UV, repeated with different seeds."""
    base = []
    for texel in range(4):
        for direction, (incoming, outgoing) in enumerate(DIRECTIONS):
            base.append(dict(texel=texel, direction=direction,
                             uv=(.25 + .5 * (texel % 2), .25 + .5 * (texel // 2)),
                             incoming=tuple(map(f32, incoming)), outgoing=tuple(map(f32, outgoing))))
    base.append(dict(base[0], uv=(1.25, -.75)))
    return [dict(base[index % len(base)], seed=17 + index * 7919) for index in range(ACTIVE_COUNT)]


def expected_outputs(inputs):
    return [reference(tuple(map(f32, COLORS[row["texel"]])), f32(ROUGHNESS[row["texel"]]),
                      row["incoming"], row["outgoing"]) for row in inputs]


def tolerance(value):
    return ABS_TOL + REL_TOL * abs(value)


def oracle_checks(inputs, expected):
    """Require positive finite outputs and distinguish omitted lobes and wrong texture inputs."""
    if len(inputs) != ACTIVE_COUNT or len(expected) != ACTIVE_COUNT:
        raise ValueError("oracle requires all 65 records")
    if not all(math.isfinite(v) and v > 0 for row in expected for v in row):
        raise ValueError("oracle outputs must all be finite and positive")
    minimum = dict.fromkeys(("missing_coat", "missing_base", "wrong_color", "wrong_roughness"), math.inf)
    for row, output in zip(inputs[:12], expected[:12]):
        texel = row["texel"]
        color, roughness = tuple(map(f32, COLORS[texel])), f32(ROUGHNESS[texel])
        for name in minimum:
            altered = reference(
                tuple(map(f32, COLORS[(texel + 1) % 4])) if name == "wrong_color" else color,
                f32(ROUGHNESS[(texel + 1) % 4]) if name == "wrong_roughness" else roughness,
                row["incoming"], row["outgoing"],
                omit={"missing_coat": "coat", "missing_base": "base"}.get(name))
            distance = max(abs(a - b) / tolerance(b) for a, b in zip(altered, output))
            minimum[name] = min(minimum[name], distance)
    if min(minimum.values()) <= 100:
        raise ValueError("oracle perturbation separation is below the frozen 100x minimum")
    return {"positive_finite_components": ACTIVE_COUNT * 4,
            "minimum_perturbation_tolerance_multiples": minimum}


def validate_ptx(text, architecture):
    """Reject incompatible fresh entry/global/stride contracts before loading their cubin."""
    requirements = {
        "target": rf"(?m)^\s*\.target\s+sm_{architecture}\b",
        "parameterless_entry": r"\.entry\s+eval_buffer\s*\(\s*\)",
        "globals_168_align8": r"\.const\s+\.align\s+8\s+\.b8\s+SLANG_globalParams\[168\]",
        "input_stride40": r"\[SLANG_globalParams\+96\];[^{}]{0,200}?mul\.(?:wide\.[su]32|lo\.s64)\s+[^;\n]+,\s*40\s*;",
        "output_stride16": r"\[SLANG_globalParams\+112\];[^{}]{0,200}?shl\.b64\s+[^;\n]+,\s*4\s*;",
    }
    for name, offset in (("material", 80), ("input", 96), ("output", 112), ("count", 160)):
        requirements[name + "_offset"] = rf"ld\.const\.u(?:32|64)\s+[^;\n]*\[SLANG_globalParams\+{offset}\]"
    entry = re.search(r"\.entry\s+eval_buffer\s*\(\s*\)\s*\{", text)
    body = ""
    if entry:
        depth = 1
        for end in range(entry.end(), len(text)):
            depth += (text[end] == "{") - (text[end] == "}")
            if depth == 0:
                body = text[entry.start():end + 1]
                break
    missing = [name for name, pattern in requirements.items()
               if not re.search(pattern, text if name in ("target", "globals_168_align8") else body)]
    if missing:
        raise ValueError("fresh PTX ABI check failed: " + ", ".join(missing))
    return list(requirements)


def reviewed_abi_hashes(path, source_sha256, modes):
    """Require complete, exact PTX identities from a separately reviewed preparation run.

    The small automatic checks deliberately do not trace arbitrary PTX register dataflow. The
    preparation artifacts expose input field loads, both material handles, and output stores for
    explicit ABI review. The execution run then demands byte-identical fresh PTX in every mode.
    """
    report = json.loads(path.read_text())
    if (report.get("status") != "prepared" or report.get("contract") != CONTRACT or
            report.get("source", {}).get("source_sha256") != source_sha256):
        raise ValueError("ABI reference must be a successful preparation of this exact source contract")
    cells = report.get("cells", [])
    wanted = {f"{backend}-o{optimization}" for backend, optimization in modes}
    if len(cells) != len(wanted) or {row.get("id") for row in cells} != wanted:
        raise ValueError("ABI reference modes are missing or duplicated")
    if any(row.get("status") != "prepared" or not re.fullmatch(r"[0-9a-f]{64}", row.get("ptx_sha256", ""))
           for row in cells):
        raise ValueError("ABI reference contains a failed or unidentified mode")
    return {row["id"]: row["ptx_sha256"] for row in cells}


def compare_outputs(data, expected):
    """Compare every component and preserve nonfinite, zero, missing and tail-write failures."""
    if len(data) != OUTPUT_COUNT * OUTPUT.size:
        return {"status": "failed", "reason": "output byte count mismatch", "bytes": len(data)}
    if len(expected) != ACTIVE_COUNT:
        raise ValueError("expected output count mismatch")
    rows, failures = [], []
    max_absolute = max_relative = max_fraction = 0
    for index, reference_values in enumerate(expected):
        actual = OUTPUT.unpack_from(data, index * OUTPUT.size)
        errors = []
        for component, (got, want) in enumerate(zip(actual, reference_values)):
            finite = math.isfinite(got)
            error = abs(got - want) if finite else None
            passed = finite and got > 0 and error <= tolerance(want)
            if not passed:
                failures.append([index, component])
            errors.append(error)
            if finite:
                max_absolute = max(max_absolute, error)
                max_relative = max(max_relative, error / abs(want))
                max_fraction = max(max_fraction, error / tolerance(want))
        rows.append({"index": index, "expected": reference_values,
                     "actual": [v if math.isfinite(v) else str(v) for v in actual],
                     "absolute_error": errors})
    tail_ok = data[ACTIVE_COUNT * OUTPUT.size:] == SENTINEL * (OUTPUT_COUNT - ACTIVE_COUNT)
    seed_repeat_ok = all(data[index * 16:(index + 1) * 16] == data[(index % 13) * 16:(index % 13 + 1) * 16]
                         for index in range(13, ACTIVE_COUNT))
    wrap_ok = data[:16] == data[12 * 16:13 * 16]
    return {"status": "passed" if not failures and tail_ok and seed_repeat_ok and wrap_ok else "failed",
            "compared_records": ACTIVE_COUNT, "compared_components": ACTIVE_COUNT * 4,
            "tail_records": OUTPUT_COUNT - ACTIVE_COUNT, "tail_sentinels_unchanged": tail_ok,
            "different_seed_outputs_identical": seed_repeat_ok, "wrapped_uv_output_identical": wrap_ok,
            "max_absolute_error": max_absolute, "max_relative_error": max_relative,
            "max_tolerance_fraction": max_fraction, "failed_components": failures, "rows": rows}


def validate_driver_log(text):
    """Require one real launch, cleanup and both full-width texture handle checks."""
    handles = re.findall(r"texture=(color|roughness) handle_decimal=(\d+) handle_hex=(0x[0-9a-f]{16}) low30_fit=true nonzero=true", text)
    if len(handles) != 2 or {row[0] for row in handles} != {"color", "roughness"}:
        raise ValueError("missing or duplicate valid texture handles")
    for _, decimal, hexadecimal in handles:
        value = int(decimal)
        if value != int(hexadecimal, 16) or not value or value & ~0x3fffffff:
            raise ValueError("texture handle did not round-trip through low30")
    execution = "execution launches=1 active=65 output_capacity=128 global_bytes=168 input_stride=40"
    if text.splitlines().count(execution) != 1 or text.splitlines().count("cleanup=PASS") != 1:
        raise ValueError("missing real execution or successful cleanup")
    device = re.search(r"cuda_header_version=(\d+) driver_api_version=(\d+) device_ordinal=0 device=(.+) sm=(\d+)", text)
    if not device:
        raise ValueError("missing CUDA device identity")
    return {"launches": 1, "active_records": ACTIVE_COUNT, "output_capacity": OUTPUT_COUNT,
            "cuda_header_version": int(device[1]), "driver_api_version": int(device[2]),
            "device": device[3], "device_ordinal": 0, "compute_capability": int(device[4]),
            "texture_handles": {name: {"decimal": decimal, "hex": hexadecimal}
                                for name, decimal, hexadecimal in handles}}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--slangc", type=Path, required=True)
    parser.add_argument("--provider", type=Path, required=True)
    parser.add_argument("--cuda-root", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    phase = parser.add_mutually_exclusive_group(required=True)
    phase.add_argument("--prepare-only", action="store_true",
                       help="compile all modes without GPU execution, for explicit ABI review")
    phase.add_argument("--abi-reference", type=Path,
                       help="results.json from an explicitly reviewed --prepare-only run")
    parser.add_argument("--cxx", default="c++")
    parser.add_argument("--timeout", type=int, default=180)
    args = parser.parse_args()
    if args.timeout <= 0 or args.timeout > 1800:
        parser.error("timeout must be between 1 and 1800 seconds")
    for name in ("slangc", "provider", "cuda_root", "output"):
        setattr(args, name, getattr(args, name).resolve())
    if args.output.exists():
        parser.error("output directory already exists; use a new directory to preserve every attempt")
    args.output.mkdir(parents=True)
    report = {"schema": 1, "contract": CONTRACT, "status": "infrastructure-failed", "cells": [],
              "started_utc": datetime.now(timezone.utc).isoformat(), "platform": platform.platform(),
              "absolute_tolerance": ABS_TOL, "relative_tolerance": REL_TOL,
              "scope": "unchanged eval_buffer, two synthetic textures, finite front-facing reflection",
              "limitations": ["no original assets", "no LUT read coverage", "no sample_buffer",
                              "no arbitrary graph or GPU performance claim"]}
    def save():
        (args.output / "results.json").write_text(json.dumps(report, indent=2, allow_nan=False) + "\n")
    save()
    try:
        if platform.system() != "Linux" or "microsoft" in platform.release().lower() or sys.byteorder != "little":
            raise ValueError("this bounded driver harness requires native little-endian Linux")
        corpus = load_tool("material_corpus", "issue-nvvm-backend/run-complex-corpus.py")
        toolkit = corpus.load_toolkit_helpers()
        manifest_path = REPO / "issue-nvvm-backend/complex-corpus.manifest.json"
        manifest = corpus.read_workloads(manifest_path, toolkit)
        workload = next(row for row in manifest["workloads"] if row["name"] == "tiled-brass-material")
        if "eval_buffer" not in workload["entry_points"]:
            raise ValueError("registered eval_buffer entry is absent")
        report.update(source=workload, architecture=manifest["architecture"], manifest=str(manifest_path))
        abi_hashes = None
        if args.abi_reference:
            args.abi_reference = args.abi_reference.resolve()
            abi_hashes = reviewed_abi_hashes(args.abi_reference, workload["source_sha256"], corpus.MODES)
            report["reviewed_abi_reference"] = {"path": str(args.abi_reference),
                                               "sha256": toolkit.sha256(args.abi_reference),
                                               "ptx_sha256": abi_hashes}
        libnvvm = toolkit.select_libnvvm(args.cuda_root)
        nvrtc = (args.cuda_root / "lib64/libnvrtc.so").resolve()
        compiler_library = args.slangc.parent.parent / "lib/libslang-compiler.so"
        driver_source = REPO / "extras/nvvm-material-runtime-driver.cpp"
        cxx = shutil.which(args.cxx)
        if not cxx:
            raise ValueError("C++ compiler is unavailable: " + args.cxx)
        ptxas = args.cuda_root / "bin/ptxas"
        nvcc = args.cuda_root / "bin/nvcc"
        artifacts = (args.slangc, compiler_library, args.provider, libnvvm, nvrtc, ptxas,
                     args.cuda_root / "nvvm/libdevice/libdevice.10.bc", args.cuda_root / "include/cuda.h",
                     args.cuda_root / "include/cudaTypedefs.h", driver_source, Path(__file__),
                     manifest_path, REPO / workload["source"], Path(cxx),
                     REPO / "issue-nvvm-backend/run-complex-corpus.py",
                     REPO / "extras/validate-nvvm-toolkit.py")
        report["artifact_sha256"] = {str(path): toolkit.sha256(toolkit.require_file(path)) for path in artifacts}
        environment = dict(os.environ, CUDA_PATH=str(args.cuda_root), CUDA_HOME=str(args.cuda_root),
                           LIBNVVM_HOME=str(args.cuda_root), SLANG_NVVM_BUILDER_PATH=str(args.provider))
        environment["LD_LIBRARY_PATH"] = os.pathsep.join(
            [str(compiler_library.parent), str(libnvvm.parent), str(nvrtc.parent),
             environment.get("LD_LIBRARY_PATH", "")])
        report["environment"] = {key: environment.get(key) for key in
                                 ("CUDA_PATH", "CUDA_HOME", "LIBNVVM_HOME", "SLANG_NVVM_BUILDER_PATH",
                                  "LD_LIBRARY_PATH", "CUDA_VISIBLE_DEVICES")}
        for name, command in (("compiler_version", [str(args.slangc), "-version"]),
                              ("toolkit_version", [str(nvcc), "--version"]),
                              ("host_compiler_version", [cxx, "--version"])):
            report[name] = subprocess.check_output(command, env=environment, text=True,
                                                   stderr=subprocess.STDOUT, timeout=args.timeout).strip()
        driver_version = Path("/proc/driver/nvidia/version")
        report["driver_version"] = driver_version.read_text() if driver_version.exists() else None
        helper = args.output / "material-driver"
        report["helper_build"] = toolkit.run(
            [cxx, "-std=c++17", "-Wall", "-Wextra", "-Werror", "-O2",
             "-I" + str(args.cuda_root / "include"), str(driver_source), "-ldl", "-o", str(helper)],
            args.output / "helper-build.log", environment, args.timeout)
        save()
        if report["helper_build"]["return_code"] != 0:
            raise ValueError("material driver helper build failed")
        report["artifact_sha256"][str(helper)] = toolkit.sha256(helper)
        inputs = cases()
        expected = expected_outputs(inputs)
        report["oracle_checks"] = oracle_checks(inputs, expected)
        payloads = {
            "inputs.bin": b"".join(INPUT.pack(*row["uv"], *row["incoming"], *row["outgoing"], row["seed"]) for row in inputs),
            "color.bin": struct.pack("<16f", *(v for color in COLORS for v in color)),
            "roughness.bin": struct.pack("<16f", *(v for roughness in ROUGHNESS for v in (roughness, 0, 0, 1))),
        }
        # Persist the oracle and its tolerance before any shader launch or observed GPU output.
        oracle_path = args.output / "expected.json"
        oracle_path.write_text(json.dumps(dict(inputs=inputs, expected=expected, absolute_tolerance=ABS_TOL,
                                              relative_tolerance=REL_TOL), indent=2, allow_nan=False) + "\n")
        report["oracle_sha256"] = toolkit.sha256(oracle_path)
        for backend, optimization in corpus.MODES:
            report["cells"].append(dict(id=f"{backend}-o{optimization}", backend=backend,
                optimization=optimization, architecture=manifest["architecture"], entry="eval_buffer", status="pending"))
        report["status"] = "running"
        save()
        for cell in report["cells"]:
            directory = args.output / cell["id"]
            directory.mkdir()
            try:
                for name, data in payloads.items():
                    (directory / name).write_bytes(data)
                cell["input_sha256"] = {name: toolkit.sha256(directory / name) for name in payloads}
                ptx, cubin = directory / "shader.ptx", directory / "shader.cubin"
                cell["compile"] = toolkit.run(corpus.compile_command(cell, workload, args) + ["-o", str(ptx)],
                                               directory / "compile.log", environment, args.timeout)
                cell["status"] = "compile-failed"
                if cell["compile"]["return_code"] == 0:
                    cell["ptx_abi_checks"] = validate_ptx(toolkit.require_file(ptx).read_text(), cell["architecture"])
                    cell["ptx_sha256"] = toolkit.sha256(ptx)
                    if abi_hashes is not None and cell["ptx_sha256"] != abi_hashes[cell["id"]]:
                        raise ValueError("fresh PTX differs from the reviewed ABI artifact")
                    cell["assembly"] = toolkit.run([str(ptxas), "-v", f"-arch=sm_{cell['architecture']}", str(ptx), "-o", str(cubin)],
                                                    directory / "assembly.log", environment, args.timeout)
                    cell["status"] = "assembly-failed"
                    if cell["assembly"]["return_code"] == 0:
                        cell["cubin_sha256"] = toolkit.sha256(toolkit.require_file(cubin))
                        if args.prepare_only:
                            cell["status"] = "prepared"
                        else:
                            cell["execution"] = toolkit.run([str(helper), str(cubin), str(directory)],
                                                             directory / "execution.log", environment, args.timeout)
                            cell["status"] = "execution-failed"
                            if cell["execution"]["return_code"] == 0:
                                cell["runtime"] = validate_driver_log((directory / "execution.log").read_text())
                                cell["comparison"] = compare_outputs((directory / "outputs.bin").read_bytes(), expected)
                                cell["status"] = "passed" if cell["comparison"]["status"] == "passed" else "output-mismatch"
                cell["runtime_artifact_sha256"] = {name: toolkit.sha256(directory / name)
                    for name in ("outputs.bin", "material.bin", "globals.bin") if (directory / name).exists()}
            except (OSError, ValueError) as error:
                cell.update(status="infrastructure-failed", error=str(error))
            print(f"{cell['id']}: {cell['status']}", flush=True)
            save()
        # A mutable input or executable changing during a run invalidates the recorded provenance.
        if any(toolkit.sha256(Path(path)) != digest for path, digest in report["artifact_sha256"].items()):
            raise ValueError("an input or executable changed during validation")
        success = "prepared" if args.prepare_only else "passed"
        report["status"] = success if all(cell["status"] == success for cell in report["cells"]) else "failed"
        save()
        return 0 if report["status"] == success else 1
    except (OSError, ValueError, KeyError, TypeError, StopIteration, subprocess.SubprocessError) as error:
        report.update(status="infrastructure-failed", error=str(error))
        save()
        print(str(error), file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
