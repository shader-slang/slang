#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""GPU timing with prerecorded commands and independent NVML clock sampling."""
import ctypes
import hashlib
import os
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import time


def sample_clocks(path):
    nvml = ctypes.CDLL("libnvidia-ml.so.1")
    assert nvml.nvmlInit_v2() == 0
    count = ctypes.c_uint()
    assert (
        nvml.nvmlDeviceGetCount_v2(ctypes.byref(count)) == 0 and count.value == 1
    ), "This harness requires a single visible GPU"
    handle = ctypes.c_void_p()
    assert nvml.nvmlDeviceGetHandleByIndex_v2(0, ctypes.byref(handle)) == 0
    with open(path, "w", buffering=1) as output:
        print("READY", flush=True)
        while True:
            values = []
            for kind in [0, 1, 2]:
                value = ctypes.c_uint()
                assert (
                    nvml.nvmlDeviceGetClockInfo(handle, kind, ctypes.byref(value)) == 0
                )
                values.append(value.value)
            state = ctypes.c_uint()
            assert nvml.nvmlDeviceGetPerformanceState(handle, ctypes.byref(state)) == 0
            output.write(json.dumps([time.perf_counter(), *values, state.value]) + "\n")
            time.sleep(0.01)


def measure(device, encode, iterations=500, warmup_min=300, warmup_seconds_min=3.0):
    import numpy as np
    import slangpy as spy

    if iterations <= 0 or warmup_min <= 0 or warmup_seconds_min <= 0:
        raise ValueError("Iteration counts and warmup duration must be positive")
    pool = device.create_query_pool(spy.QueryType.timestamp, iterations * 2)
    encoder = device.create_command_encoder()
    for i in range(iterations):
        encoder.write_timestamp(pool, i * 2)
        encode(encoder, i)
        encoder.write_timestamp(pool, i * 2 + 1)
    measured_command = encoder.finish()
    with tempfile.TemporaryDirectory(prefix="nvvm-perf-clocks-") as folder:
        path = Path(folder) / "samples.jsonl"
        sampler = subprocess.Popen(
            [sys.executable, str(Path(__file__).resolve()), str(path)],
            stdout=subprocess.PIPE,
            text=True,
        )
        try:
            assert (
                sampler.stdout.readline().strip() == "READY"
            ), "NVML sampler did not start"
            warmup_started = time.perf_counter()
            warmup_count = 0
            while (
                warmup_count < warmup_min
                or time.perf_counter() - warmup_started < warmup_seconds_min
            ):
                encoder = device.create_command_encoder()
                for i in range(64):
                    encode(encoder, warmup_count + i)
                device.submit_command_buffer(encoder.finish())
                device.wait_for_idle()
                warmup_count += 64
            warmup_seconds = time.perf_counter() - warmup_started
            measurement_start = time.perf_counter()
            device.submit_command_buffer(measured_command)
            device.wait_for_idle()
            measurement_end = time.perf_counter()
            assert sampler.poll() is None, "NVML sampler exited during measurement"
        finally:
            sampler.terminate()
            sampler.wait(timeout=10)
            sampler.stdout.close()
        all_samples = [json.loads(line) for line in path.read_text().splitlines()]
    clock_samples = [
        s for s in all_samples if measurement_start <= s[0] <= measurement_end
    ]
    assert clock_samples, "No in-measurement clock samples; increase iterations"
    timestamps = pool.get_timestamp_results(0, iterations * 2)
    gpu_ms = [
        (timestamps[2 * i + 1] - timestamps[2 * i]) * 1000 for i in range(iterations)
    ]
    assert all(np.isfinite(gpu_ms)) and min(gpu_ms) > 0
    return {
        "clock_summary": {
            "samples": len(clock_samples),
            "sm_mhz": sorted({s[2] for s in clock_samples}),
            "memory_mhz": sorted({s[3] for s in clock_samples}),
            "pstate": sorted({s[4] for s in clock_samples}),
        },
        "gpu_ms_per_iteration": gpu_ms,
        "batch_submit_wait_ms": (measurement_end - measurement_start) * 1000,
        "submission": "prerecorded-single-command",
        "iterations": iterations,
        "warmup_iterations": warmup_count,
        "warmup_seconds": warmup_seconds,
        "median_gpu_ms": float(np.median(gpu_ms)),
        "p10_gpu_ms": float(np.percentile(gpu_ms, 10)),
        "p90_gpu_ms": float(np.percentile(gpu_ms, 90)),
        "clock_samples_perf_graphics_sm_memory_pstate": clock_samples,
        "warmup_clock_samples": [
            s for s in all_samples if warmup_started <= s[0] < measurement_start
        ],
        "measurement_interval": [measurement_start, measurement_end],
    }


def record_identity(output_path, backend):
    import falcor2
    import slangpy
    import slangpy.slangpy_ext
    import falcor2.falcor2_ext

    libraries = {}
    for line in Path("/proc/self/maps").read_text().splitlines():
        fields = line.split()
        if len(fields) < 6:
            continue
        path = fields[-1]
        if any(
            name in path
            for name in (
                "libsgl.",
                "libslang-",
                "slangpy_ext.",
                "libfalcor2.",
                "falcor2_ext.",
                "libnvvm.",
                "libnvrtc.",
                "libnvoptix.",
                "libvulkan.",
                "libnvidia-glvkspirv.",
                "libGLX_nvidia.",
            )
        ):
            file = Path(path)
            if file.is_file() and path not in libraries:
                libraries[path] = hashlib.sha256(file.read_bytes()).hexdigest()
    build_sources = {}
    for library in libraries:
        if Path(library).name.startswith("libsgl."):
            cache = Path(library).parent.parent / "CMakeCache.txt"
            if cache.is_file():
                for line in cache.read_text().splitlines():
                    if line.startswith(
                        (
                            "slang-rhi_SOURCE_DIR:",
                            "SLANG_RHI_SLANG_BINARY_DIR:",
                            "SLANG_RHI_SLANG_INCLUDE_DIR:",
                        )
                    ):
                        key, value = line.split("=", 1)
                        build_sources[key] = value
    report = {
        "python": sys.executable,
        "backend": backend,
        "status": "render-and-oracle-complete; process exit checked separately",
        "packages": {
            m.__name__: m.__file__
            for m in (falcor2, slangpy, slangpy.slangpy_ext, falcor2.falcor2_ext)
        },
        "libraries_sha256": libraries,
        "build_sources": build_sources,
    }
    Path(output_path).write_text(json.dumps(report, indent=2) + "\n")


if __name__ == "__main__":
    sample_clocks(sys.argv[1])
