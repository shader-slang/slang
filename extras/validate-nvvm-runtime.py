#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""Require actual CUDA execution of four existing NVVM/NVRTC differential fixtures."""

import argparse
import ctypes
import importlib.util
import json
import os
from pathlib import Path
import platform
import subprocess
import sys
import time


REPO = Path(__file__).resolve().parents[1]
FIXTURES = (
    "nvvmSlangCUDAExecutionRuntimeMatchesNVRTC",
    "nvvmSlangSharedMemoryRuntimeMatchesNVRTC",
    "nvvmSlangRelaxedGlobalI32AtomicAddRuntimeMatchesNVRTC",
    "nvvmSlangWaveReadLaneAtUIntRuntimeMatchesNVRTC",
)


def load_tool(name, path):
    """Reuse the existing census classification and toolkit artifact checks."""
    spec = importlib.util.spec_from_file_location(name, REPO / path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def cuda_device_metadata():
    """Query visible device zero, which the existing runtime fixtures execute on."""
    library = ctypes.WinDLL("nvcuda.dll") if os.name == "nt" else ctypes.CDLL("libcuda.so.1")
    signatures = {
        "cuInit": [ctypes.c_uint],
        "cuDriverGetVersion": [ctypes.POINTER(ctypes.c_int)],
        "cuDeviceGetCount": [ctypes.POINTER(ctypes.c_int)],
        "cuDeviceGet": [ctypes.POINTER(ctypes.c_int), ctypes.c_int],
        "cuDeviceGetName": [ctypes.c_void_p, ctypes.c_int, ctypes.c_int],
        "cuDeviceGetAttribute": [ctypes.POINTER(ctypes.c_int), ctypes.c_int, ctypes.c_int],
    }
    for name, arguments in signatures.items():
        function = getattr(library, name)
        function.argtypes = arguments
        function.restype = ctypes.c_int

    def call(name, *arguments):
        result = getattr(library, name)(*arguments)
        if result:
            raise RuntimeError(f"CUDA driver {name} failed with status {result}")

    call("cuInit", 0)
    count = ctypes.c_int()
    call("cuDeviceGetCount", ctypes.byref(count))
    if count.value < 1:
        raise RuntimeError("no visible CUDA device")
    device, version = ctypes.c_int(), ctypes.c_int()
    call("cuDeviceGet", ctypes.byref(device), 0)
    call("cuDriverGetVersion", ctypes.byref(version))
    name = ctypes.create_string_buffer(256)
    call("cuDeviceGetName", name, len(name), device.value)
    major, minor = ctypes.c_int(), ctypes.c_int()
    call("cuDeviceGetAttribute", ctypes.byref(major), 75, device.value)
    call("cuDeviceGetAttribute", ctypes.byref(minor), 76, device.value)
    return {
        "name": name.value.decode("utf-8", errors="replace"),
        "ordinal": 0,
        "visible_device_count": count.value,
        "compute_capability": major.value * 10 + minor.value,
        "driver_api_version": version.value,
        "cuda_visible_devices": os.environ.get("CUDA_VISIBLE_DEVICES"),
    }


def require_file(path, description):
    """Reject missing prerequisites before a runtime fixture could skip them."""
    if not path.is_file():
        raise RuntimeError(f"missing {description}: {path}")
    return path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--config", choices=["Debug", "Release", "RelWithDebInfo"], default="Release")
    parser.add_argument("--bin-dir", type=Path)
    parser.add_argument("--provider", type=Path, help="provider directory or exact module")
    parser.add_argument("--cuda-path", type=Path, default=os.environ.get("CUDA_PATH"))
    parser.add_argument("--architecture", type=int, choices=[70, 80, 90], default=80)
    parser.add_argument("--output", type=Path, default=REPO / "build/nvvm-runtime-validation")
    parser.add_argument("--host", choices=["native", "linux"], default="native",
                        help="WSL requires Windows Python by default; explicitly choose linux for WSL CUDA")
    parser.add_argument("--timeout", type=float, default=120, help="seconds allowed per fixture")
    args = parser.parse_args()
    if args.timeout <= 0:
        parser.error("--timeout must be positive")
    output = args.output.resolve()
    output.mkdir(parents=True, exist_ok=True)
    report = {
        "schema": 1,
        "status": "blocked",
        "architecture": args.architecture,
        "optimization": "compiler-default (explicit O0/O3 coverage belongs to the compute census)",
        "platform": platform.platform(),
        "expected_fixtures": list(FIXTURES),
        "results": [],
    }
    exit_code = 2
    try:
        if platform.system() == "Linux" and "microsoft" in platform.release().lower() and args.host != "linux":
            raise RuntimeError("On WSL, run with Windows Python and Windows binaries; or explicitly choose --host linux to query the WSL CUDA driver")
        if platform.system() not in ("Linux", "Windows"):
            raise RuntimeError("CUDA runtime validation requires a native Linux or Windows host")
        suffix = ".exe" if os.name == "nt" else ""
        bin_dir = (args.bin_dir or REPO / "build" / args.config / "bin").resolve()
        runner = require_file(bin_dir / ("slang-test" + suffix), "Slang test runner")
        compiler = require_file(bin_dir / ("slangc" + suffix), "Slang compiler")
        provider = (args.provider or bin_dir).resolve()
        filename = "slang-llvm-nvvm.dll" if os.name == "nt" else "libslang-llvm-nvvm.so"
        if provider.is_dir():
            provider = provider / filename
        require_file(provider, "NVVM provider")
        toolkit_tools = load_tool("nvvm_toolkit", "extras/validate-nvvm-toolkit.py")
        report.update({"test_runner": str(runner), "compiler": str(compiler), "provider": str(provider),
                       "compiler_sha256": toolkit_tools.sha256(compiler),
                       "provider_sha256": toolkit_tools.sha256(provider)})
        if args.cuda_path is None:
            raise RuntimeError("missing CUDA Toolkit; pass --cuda-path or set CUDA_PATH")
        cuda_root = args.cuda_path.resolve()
        nvcc = require_file(cuda_root / "bin" / ("nvcc" + suffix), "CUDA compiler/version tool")
        require_file(cuda_root / "nvvm/libdevice/libdevice.10.bc", "CUDA libdevice")
        libnvvm = toolkit_tools.select_libnvvm(cuda_root)
        if os.name == "nt":
            candidates = sorted(set((cuda_root / "bin").glob("nvrtc64_*.dll")) |
                                set((cuda_root / "bin/x64").glob("nvrtc64_*.dll")))
            if len(candidates) != 1:
                raise RuntimeError("expected exactly one NVRTC DLL in the selected toolkit")
            nvrtc = candidates[0].resolve()
        else:
            nvrtc = require_file(cuda_root / "lib64/libnvrtc.so", "CUDA NVRTC library").resolve()
        report["toolkit_libraries"] = {
            "required_libnvvm": str(libnvvm), "required_nvrtc": str(nvrtc),
            "libnvvm_sha256": toolkit_tools.sha256(libnvvm),
            "nvrtc_sha256": toolkit_tools.sha256(nvrtc),
        }
        toolkit_version = subprocess.check_output([str(nvcc), "--version"], text=True, timeout=args.timeout)
        report["toolkit"] = {"path": str(cuda_root), "nvcc_version": toolkit_version.strip()}
        report["compiler_version"] = subprocess.check_output(
            [str(compiler), "-version"], text=True, stderr=subprocess.STDOUT, timeout=args.timeout
        ).strip()
        device = cuda_device_metadata()
        report["device"] = device
        if device["compute_capability"] < args.architecture:
            raise RuntimeError(f"device sm_{device['compute_capability']} cannot execute selected sm_{args.architecture}")
        environment = dict(os.environ, CUDA_PATH=str(cuda_root), CUDA_HOME=str(cuda_root),
                           LIBNVVM_HOME=str(cuda_root), SLANG_NVVM_BUILDER_PATH=str(provider),
                           SLANG_NVVM_TEST_ARCH=str(args.architecture))
        search_path = "PATH" if os.name == "nt" else "LD_LIBRARY_PATH"
        environment[search_path] = os.pathsep.join(
            [str(libnvvm.parent), str(nvrtc.parent), environment.get(search_path, "")]
        )
        census = load_tool("nvvm_compute_census", "issue-nvvm-backend/run-compute-census.py")
        report["status"] = "failed"
        for fixture in FIXTURES:
            command = [str(runner), "-disable-retries", "slang-unit-test-tool/" + fixture + ".internal"]
            start = time.perf_counter()
            result = subprocess.run(command, cwd=REPO, env=environment, text=True,
                                    stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                    timeout=args.timeout, check=False)
            log = output / (fixture + ".log")
            log.write_text(result.stdout, encoding="utf-8")
            classification, diagnostic, _ = census._classify_result(result.returncode, result.stdout, "nvvm-runtime")
            record = {"fixture": fixture, "command": command, "return_code": result.returncode,
                      "classification": classification, "diagnostic": diagnostic,
                      "execution_counts": census.execution_counts(result.stdout), "log": str(log),
                      "elapsed_ms": round((time.perf_counter() - start) * 1000)}
            report["results"].append(record)
            print(f"{fixture}: {classification}", flush=True)
        if len(report["results"]) == len(FIXTURES) and all(
            item["classification"] == "correct" for item in report["results"]
        ):
            report["status"] = "passed"
            exit_code = 0
        else:
            report["reason"] = "every fixture must actually execute and pass with no ignored tests"
            exit_code = 1
    except (OSError, ValueError, RuntimeError, AttributeError, subprocess.SubprocessError) as error:
        report["reason"] = str(error)
        print("GPU validation " + report["status"] + ": " + str(error), file=sys.stderr)
    finally:
        (output / "results.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print("Report: " + str(output / "results.json"), flush=True)
    return exit_code


if __name__ == "__main__":
    sys.exit(main())
