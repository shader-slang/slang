#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""Require NVVM-to-PTX compilation and ptxas assembly for every selected matrix cell.

This is a compile/assembly gate; it does not run kernels or require a GPU. Every requested
architecture is mandatory, including one that the selected toolkit ultimately rejects.
"""

import argparse
from collections import Counter
from datetime import datetime, timezone
import hashlib
import itertools
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import time


REPO = Path(__file__).resolve().parents[1]
# These established file tests cover distinct parts of the selected compute backend. Their test
# directives remain untouched: this gate supplies its own explicit architecture and optimization.
WORKLOADS = [
    ("core-execution", "computeMain"),
    ("mixed-numeric", "computeMain"),
    ("native-float-surface", "computeMain"),
    ("float64-values", "computeMain"),
    ("thread-local-global-context", "computeMain"),
    ("half-values", "computeMain"),
    ("float-matrix-values", "computeMain"),
    ("helper-copyable-values", "computeMain"),
    ("conventional-global-multi-resource", "computeMain"),
]


def run(command, log, environment, timeout):
    """Retain each subprocess diagnostic and distinguish failure to start from its exit code."""
    started = time.monotonic()
    record = {"command": [str(arg) for arg in command], "log": str(log), "return_code": None}
    try:
        with log.open("w", encoding="utf-8") as stream:
            completed = subprocess.run(
                record["command"], cwd=REPO, env=environment, stdout=stream,
                stderr=subprocess.STDOUT, timeout=timeout, check=False,
            )
        record["return_code"] = completed.returncode
    except (OSError, subprocess.TimeoutExpired) as error:
        record["error"] = str(error)
    record["elapsed_seconds"] = round(time.monotonic() - started, 3)
    return record


def require_file(path):
    """Reject missing or empty inputs before any matrix cell can be called successful."""
    if not path.is_file() or path.stat().st_size == 0:
        raise ValueError("Required nonempty file is missing: " + str(path))
    return path


def sha256(path):
    """Identify the actual compiler and provider used by this validation run."""
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def select_libnvvm(root):
    """Select a concrete library from this toolkit, avoiding automatic search-path fallback."""
    if os.name == "nt":
        candidates = sorted((root / "nvvm/bin").glob("nvvm64_*.dll"))
        if len(candidates) != 1:
            raise ValueError("Expected exactly one libNVVM DLL below " + str(root / "nvvm/bin"))
        return require_file(candidates[0]).resolve()
    return require_file(root / "nvvm/lib64/libnvvm.so").resolve()


def validate_cell(cell, args, environment, libnvvm):
    """Compile one shader, validate its actual PTX target, and assemble only its fresh output."""
    source = REPO / cell["source"]
    ptx = args.output / (cell["id"] + ".ptx")
    cubin = args.output / (cell["id"] + ".cubin")
    cell.update({"ptx": str(ptx), "cubin": str(cubin)})
    # An earlier successful run must not supply a missing output for this run.
    ptx.unlink(missing_ok=True)
    cubin.unlink(missing_ok=True)
    require_file(source)
    cell["source_sha256"] = sha256(source)
    architecture = cell["architecture"]
    compile_command = [
        args.slangc, source, "-entry", cell["entry"], "-stage", "compute", "-target", "ptx",
        "-emit-cuda-via-nvvm", "-nvvm-path", libnvvm,
        "-capability", f"cuda_sm_{architecture // 10}_{architecture % 10}",
        f"-O{cell['optimization']}", "-o", ptx,
    ]
    cell["compile"] = run(
        compile_command, args.output / (cell["id"] + ".compile.log"), environment, args.timeout,
    )
    if cell["compile"]["return_code"] != 0:
        cell["status"] = "compile-failed"
        return
    require_file(ptx)
    text = ptx.read_text(encoding="utf-8")
    target = re.search(r"^\s*\.target\s+sm_(\d+)\b", text, re.MULTILINE)
    cell["actual_ptx_target"] = int(target.group(1)) if target else None
    if cell["actual_ptx_target"] != architecture:
        cell.update(
            status="invalid-ptx", error="PTX target does not match the required architecture",
        )
        return
    if not re.search(r"\.entry\s+" + re.escape(cell["entry"]) + r"\s*\(", text):
        cell.update(status="invalid-ptx", error="Required kernel entry is missing from PTX")
        return
    cell["assemble"] = run(
        [args.ptxas, f"-arch=sm_{architecture}", ptx, "-o", cubin],
        args.output / (cell["id"] + ".assemble.log"), environment, args.timeout,
    )
    if cell["assemble"]["return_code"] != 0:
        cell["status"] = "assemble-failed"
        return
    require_file(cubin)
    cell.update(
        status="passed", ptx_bytes=ptx.stat().st_size, cubin_bytes=cubin.stat().st_size,
        ptx_sha256=sha256(ptx), cubin_sha256=sha256(cubin),
    )


def parse_arguments():
    """Require explicit tool and architecture inputs so host defaults cannot change the matrix."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--slangc", type=Path, required=True)
    parser.add_argument(
        "--provider", type=Path, required=True, help="compiler-matched provider file",
    )
    parser.add_argument("--cuda-root", type=Path, required=True)
    parser.add_argument("--architectures", type=int, nargs="+", required=True, help="e.g. 80 90")
    parser.add_argument("--optimizations", type=int, nargs="+", choices=[0, 3], default=[0, 3])
    parser.add_argument("--expected-toolkit", help="required major.minor version, e.g. 13.4")
    parser.add_argument("--timeout", type=int, default=120, help="per-process timeout in seconds")
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.timeout <= 0:
        parser.error("--timeout must be positive")
    if any(architecture < 10 or architecture > 999 for architecture in args.architectures):
        parser.error("architectures must be numeric SM versions between 10 and 999")
    for values in (args.architectures, args.optimizations):
        if len(set(values)) != len(values):
            parser.error("architecture and optimization lists must not contain duplicates")
    if args.expected_toolkit and not re.fullmatch(r"\d+\.\d+", args.expected_toolkit):
        parser.error("--expected-toolkit must be a major.minor version")
    for name in ("slangc", "provider", "cuda_root", "output"):
        setattr(args, name, getattr(args, name).resolve())
    args.ptxas = args.cuda_root / "bin" / ("ptxas.exe" if os.name == "nt" else "ptxas")
    return args


def main():
    args = parse_arguments()
    args.output.mkdir(parents=True, exist_ok=True)
    (args.output / "results.json").unlink(missing_ok=True)
    cells = [
        {
            "id": f"{name}-sm{architecture}-o{optimization}",
            "source": f"tests/cuda/nvvm-{name}.slang", "entry": entry,
            "architecture": architecture, "optimization": optimization,
            "status": "infrastructure-failed",
        }
        for (name, entry), architecture, optimization in itertools.product(
            WORKLOADS, args.architectures, args.optimizations,
        )
    ]
    document = {
        "schema": 1, "started_utc": datetime.now(timezone.utc).isoformat(),
        "gpu_execution": False, "slangc": str(args.slangc), "provider": str(args.provider),
        "cuda_root": str(args.cuda_root), "required_cell_count": len(cells), "cells": cells,
    }
    environment = os.environ.copy()
    environment.update({
        "CUDA_PATH": str(args.cuda_root), "CUDA_HOME": str(args.cuda_root),
        "LIBNVVM_HOME": str(args.cuda_root),
        "SLANG_NVVM_BUILDER_PATH": str(args.provider),
    })
    try:
        require_file(args.slangc)
        require_file(args.provider)
        require_file(args.ptxas)
        libnvvm = select_libnvvm(args.cuda_root)
        libdevice = require_file(args.cuda_root / "nvvm/libdevice/libdevice.10.bc")
        version_file = require_file(args.cuda_root / "version.json")
        version = json.loads(version_file.read_text(encoding="utf-8"))["cuda"]["version"]
        if not isinstance(version, str) or not re.fullmatch(r"\d+\.\d+\.\d+", version):
            raise ValueError("CUDA version.json must provide cuda.version as major.minor.patch")
        major_minor = ".".join(version.split(".")[:2])
        if args.expected_toolkit and major_minor != args.expected_toolkit:
            raise ValueError(f"Expected CUDA {args.expected_toolkit}, found {version}")
        document.update({
            "toolkit_version": version, "libnvvm": str(libnvvm), "libdevice": str(libdevice),
            "compiler_sha256": sha256(args.slangc), "provider_sha256": sha256(args.provider),
            "libnvvm_sha256": sha256(libnvvm), "libdevice_sha256": sha256(libdevice),
        })
        for name, command in (("compiler", [args.slangc, "-version"]),
                              ("ptxas", [args.ptxas, "--version"])):
            record = run(command, args.output / (name + "-version.log"), environment, args.timeout)
            document[name + "_version"] = record
            if record["return_code"] != 0:
                raise ValueError("Unable to query " + name + " version; see " + record["log"])
        assembler_version = Path(document["ptxas_version"]["log"]).read_text(encoding="utf-8")
        if not re.search(r"\brelease\s+" + re.escape(major_minor) + r"\b", assembler_version):
            raise ValueError("ptxas release does not match CUDA version.json")
    except (OSError, ValueError, KeyError, TypeError) as error:
        document["error"] = str(error)
        for cell in cells:
            cell["error"] = str(error)
    else:
        for cell in cells:
            try:
                validate_cell(cell, args, environment, libnvvm)
            except (OSError, ValueError) as error:
                cell.update(status="infrastructure-failed", error=str(error))
            print(cell["id"] + ": " + cell["status"], flush=True)
    counts = dict(Counter(cell["status"] for cell in cells))
    success = len(cells) > 0 and counts.get("passed", 0) == len(cells)
    document.update(status="passed" if success else "failed", counts=counts)
    (args.output / "results.json").write_text(
        json.dumps(document, indent=2) + "\n", encoding="utf-8",
    )
    print(json.dumps({"status": document["status"], "required": len(cells), "counts": counts}))
    return 0 if success else 1


if __name__ == "__main__":
    sys.exit(main())
