#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""Replay deterministic trace/payload and math controls from the existing Falcor environment."""

import argparse
import hashlib
import json
import os
from pathlib import Path
import numpy as np
import slangpy as spy
from falcor2.testing import helpers
from slangpy.tests.slangpy_tests.test_raytracing import build_blas, build_tlas
from nvvm_perf import measure, record_identity

parser = argparse.ArgumentParser()
parser.add_argument("--workload", choices=["trace", "math"], default="trace")
parser.add_argument("--fast-math", action="store_true")
parser.add_argument("--api", choices=["cuda", "vulkan"], required=True)
parser.add_argument("--words", type=int, choices=[1, 4, 16, 32], default=1)
parser.add_argument("--capacity", type=int, default=128)
parser.add_argument("--empty", action="store_true")
parser.add_argument("--traces", type=int, default=1)
parser.add_argument("--size", type=int, default=512)
parser.add_argument("--iterations", type=int, default=1000)
parser.add_argument("--output", type=Path, required=True)

parser.add_argument("--nested", action="store_true")
args = parser.parse_args()
args.output.parent.mkdir(parents=True, exist_ok=True)
if args.capacity < 4 * args.words and not args.empty:
    parser.error("Capacity must hold every live payload word")
if args.empty and args.nested:
    parser.error("--empty and --nested are mutually exclusive")
device = helpers.get_device(
    spy.DeviceType[args.api],
    use_cache=False,
    enable_compilation_reports=True,
    floating_point_mode=(
        spy.SlangFloatingPointMode.fast
        if args.fast_math
        else spy.SlangFloatingPointMode.default
    ),
)
if args.workload == "math":
    source = (
        Path(__file__).resolve().parents[1]
        / "tests/cuda/applications/falcor-fast-math.slang"
    ).read_text()
    module = device.load_module_from_source("math_probe", source)
    program = device.link_program([module], module.entry_points)
    kernel = device.create_compute_kernel(program)
    count = args.size * args.size
    assert count % 64 == 0
    coordinates = np.linspace(0, 1, count, dtype=np.float32)
    inputs = np.zeros((count, 4), dtype=np.float32)
    inputs[:, 0] = coordinates * np.float32(2 * np.pi)
    inputs[:, 1] = np.float32(0.1) + coordinates * np.float32(0.9)
    inputs[:, 2] = np.float32(0.5) + coordinates * np.float32(3.5)
    input_buffer = device.create_buffer(
        data=inputs, struct_size=16, usage=spy.BufferUsage.shader_resource
    )
    output = device.create_buffer(
        size=count * 16, struct_size=16, usage=spy.BufferUsage.unordered_access
    )

    def encode(encoder, index):
        kernel.dispatch(
            thread_count=[count, 1, 1],
            command_encoder=encoder,
            vars={"inputs": input_buffer, "outputs": output},
        )

else:
    vertices = np.array([-10, -10, 0, 10, -10, 0, 0, 10, 0], dtype=np.float32)
    blas = build_blas(device, vertices, np.array([0, 1, 2], dtype=np.uint32))
    instances = device.create_acceleration_structure_instance_list(1)
    instances.write(
        0,
        {
            "transform": spy.float3x4.identity(),
            "instance_id": 0,
            "instance_mask": 255,
            "instance_contribution_to_hit_group_index": 0,
            "flags": spy.AccelerationStructureInstanceFlags.none,
            "acceleration_structure": blas.handle,
        },
    )
    tlas = build_tlas(device, instances)
    device.wait_for_idle()
    source = (
        Path(__file__).resolve().parents[1]
        / "tests/cuda/applications/falcor-trace-performance.slang"
    ).read_text()
    source = (
        f"#define PAYLOAD_WORDS {args.words}\n#define DO_TRACE {int(not args.empty)}\n#define NESTED_TRACE {int(args.nested)}\n"
        + source
    )
    module = device.load_module_from_source("trace_probe", source)
    entries = [
        e
        for e in module.entry_points
        if (not args.empty or e.name == "raygen")
        and (args.nested or e.name != "visibilityMiss")
    ]
    program = device.link_program([module], entries)
    pipeline = device.create_ray_tracing_pipeline(
        program=program,
        hit_groups=(
            []
            if args.empty
            else [{"hit_group_name": "hit", "closest_hit_entry_point": "closestHit"}]
        ),
        max_recursion=2 if args.nested else 1,
        max_ray_payload_size=args.capacity,
    )
    table = device.create_shader_table(
        program=program,
        ray_gen_entry_points=["raygen"],
        miss_entry_points=(
            []
            if args.empty
            else (["miss", "visibilityMiss"] if args.nested else ["miss"])
        ),
        hit_group_names=[] if args.empty else ["hit"],
    )
    output = device.create_buffer(
        size=args.size * args.size * 4,
        struct_size=4,
        usage=spy.BufferUsage.unordered_access,
    )

    def encode(encoder, index):
        with encoder.begin_ray_tracing_pass() as render:
            obj = render.bind_pipeline(pipeline, table)
            cursor = spy.ShaderCursor(obj)
            cursor.output = output
            cursor.size = args.size
            cursor.seed = 17
            if not args.empty:
                cursor.scene = tlas
                cursor.traceCount = args.traces
            render.dispatch_rays(0, [args.size, args.size, 1])


result = measure(device, encode, args.iterations)
if args.workload == "math":
    data = output.to_numpy().view(np.float32).reshape(-1, 4)
    values = inputs.astype(np.float64)
    expected = np.stack(
        [
            np.sin(values[:, 0]),
            np.cos(values[:, 0]),
            np.log(values[:, 1]),
            np.power(values[:, 1], values[:, 2]),
        ],
        axis=1,
    )
    assert np.isfinite(data).all() and np.allclose(data, expected, atol=2e-6, rtol=5e-6)
    oracle_error = float(np.max(np.abs(data - expected)))
else:
    data = output.to_numpy().view(np.uint32).reshape(-1)
    ids = np.arange(args.size * args.size, dtype=np.uint32)
    expected = ids + 17
    if not args.empty:
        expected = (
            expected + np.where(ids & 1, 7, 14 if args.nested else 3) * args.traces
        ) * args.words + args.words * (args.words - 1) // 2
    assert np.array_equal(data, expected.astype(np.uint32)), (data[:8], expected[:8])
    oracle_error = 0.0
result.update(
    {
        "backend": (
            os.environ["SLANGPY_TEST_CUDA_COMPILER"] if args.api == "cuda" else "vulkan"
        ),
        "api": args.api,
        "words": args.words,
        "capacity_bytes": args.capacity,
        "empty": args.empty,
        "traces": args.traces,
        "nested": args.nested,
        "dimensions": [args.size, args.size],
        "oracle": (
            "float64 reference, atol2e-6/rtol5e-6"
            if args.workload == "math"
            else "exact uint hit/miss checksum"
        ),
        "max_oracle_error": oracle_error,
        "workload": args.workload,
        "floating_point_mode": str(
            device.slang_session.desc.compiler_options.floating_point_mode
        ),
        "output_sha256": hashlib.sha256(data.tobytes()).hexdigest(),
        "source_sha256": hashlib.sha256(source.encode()).hexdigest(),
        "compilation_reports": device.get_compilation_reports(),
    }
)
result["harness_sha256"] = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
result["timing_helper_sha256"] = hashlib.sha256(
    Path(__file__).with_name("nvvm_perf.py").read_bytes()
).hexdigest()
result["diagnostic_environment"] = {
    k: v
    for k, v in os.environ.items()
    if k.startswith("PROBE_") or k == "NVVM_PERF_OPTIX_LIBRARY"
}
args.output.write_text(json.dumps(result, indent=2) + "\n")
print("PASS", args.output.name, result["median_gpu_ms"], flush=True)
record_identity(args.output.with_suffix(".identity.json"), result["backend"])
