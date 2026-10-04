#!/usr/bin/env python3
# SPDX-FileCopyrightText: The Khronos Group, Inc.
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""Matched single-sample path-tracer iterations; run from existing Falcor checkout."""
import argparse
import hashlib
import json
import os
import time
from pathlib import Path
import numpy as np
import slangpy as spy
import falcor2 as f2
from falcor2.testing import helpers
from falcor2.rendernodes import ReferencePathTracerNode
from nvvm_perf import measure, record_identity

parser = argparse.ArgumentParser()
parser.add_argument("--placement", choices=["early", "late"], default="late")
visibility_control = parser.add_mutually_exclusive_group()
visibility_control.add_argument("--skip-visibility", action="store_true")
visibility_control.add_argument("--miss-visibility", action="store_true")
parser.add_argument("--depth", type=int, default=6)
parser.add_argument("--no-nee", action="store_true")
parser.add_argument("--warmup", type=int, default=300)
parser.add_argument("--warmup-seconds", type=float, default=3.0)
parser.add_argument(
    "--fast-math",
    action="store_true",
    help="Diagnostic changed-precision control; output equivalence is not assumed.",
)
parser.add_argument("--api", choices=["cuda", "vulkan"], default="cuda")
parser.add_argument("--visibility", choices=["default", "trace"], default="default")
parser.add_argument("--output", type=Path, required=True)
parser.add_argument("--size", type=int, default=512)
parser.add_argument("--iterations", type=int, default=1000)

args = parser.parse_args()
args.output.parent.mkdir(parents=True, exist_ok=True)
if (args.skip_visibility or args.miss_visibility) and args.visibility != "trace":
    parser.error("Visibility ablations require --visibility trace")
started = time.perf_counter()
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
scene = f2.Scene.load(
    device, "data/assets/kronos/DamagedHelmet/glTF/DamagedHelmet.gltf"
)
light = scene.create_entity().create_component(f2.ConstantLight)
light.color = spy.float3(1.0)
position = spy.float3(0.0, 0.0, 3.0)
camera = helpers.create_test_camera(
    scene,
    width=args.size,
    height=args.size,
    fov_y=45,
    position=position,
    rotation=spy.math.quat_from_look_at(
        -spy.math.normalize(position), spy.float3(0.0, 1.0, 0.0)
    ),
)
camera.recompute()
scene.update()
scene.update()
device.wait_for_idle()
scene_seconds = time.perf_counter() - started
scene_reports = device.get_compilation_reports()
source_path = Path("slang/falcor2/rendernodes/reference_pathtracer.slang")
source = source_path.read_text()
if args.skip_visibility or args.miss_visibility:
    import falcor2.rendernodes.reference_pathtracer_node as pt_module

    begin = source.index(
        "        TraceRay(", source.index("public struct TraceRayVisibilityQuery")
    )
    end = source.index("        return !payload.visible;", begin)
    if args.skip_visibility:
        source = (
            source[:begin]
            + "        payload.visible = true; // Diagnostic: visibility traversal removed.\n"
            + source[end:]
        )
    else:
        region = source[begin:end]
        assert region.count("INSTANCE_INCLUSION_MASK_VISIBILITY_RAY") == 1
        region = region.replace(
            "INSTANCE_INCLUSION_MASK_VISIBILITY_RAY",
            "0 /* Diagnostic: guaranteed visibility miss. */",
        )
        source = source[:begin] + region + source[end:]
    copied = args.output.resolve().with_suffix(".slang")
    copied.write_text(source)
    pt_module.REFERENCE_MODULE_PATH = str(copied)
pt = ReferencePathTracerNode.create(device)
pt.max_depth = args.depth
pt.enable_nee = not args.no_nee
if args.visibility == "trace":
    from falcor2.rendernodes.reference_pathtracer_node import (
        VisibilityRayMode,
        VisibilityRayPlacement,
    )

    pt.visibility_ray_mode = VisibilityRayMode.trace_ray
    pt.visibility_ray_placement = VisibilityRayPlacement[args.placement]
color = spy.Tensor.empty(device, (args.size, args.size), spy.float4)
started = time.perf_counter()
pt._render(scene, camera, color, iteration=0)
device.wait_for_idle()
first_render_seconds = time.perf_counter() - started
initial = color.to_numpy()
assert np.isfinite(initial).all() and initial[..., :3].max() > 0
args.output.parent.mkdir(parents=True, exist_ok=True)
np.save(args.output.with_suffix(".first.npy"), initial)
reports = device.get_compilation_reports()


def encode(encoder, i):
    pt._render(scene, camera, color, iteration=i, cmd=encoder)


timing = measure(device, encode, args.iterations, args.warmup, args.warmup_seconds)
last = color.to_numpy()
assert np.isfinite(last).all() and last[..., :3].max() > 0
np.save(args.output.with_suffix(".last.npy"), last)
result = {
    "backend": (
        os.environ["SLANGPY_TEST_CUDA_COMPILER"]
        if args.api == "cuda"
        else "vulkan-" + args.visibility
    ),
    "api": args.api,
    "dimensions": [args.size, args.size],
    "scene": "DamagedHelmet",
    "experiment": {
        "depth": args.depth,
        "nee": not args.no_nee,
        "placement": pt.visibility_ray_placement.name,
        "skip_visibility": args.skip_visibility,
        "miss_visibility": args.miss_visibility,
    },
    "camera_position": [0, 0, 3],
    "fov_y": 45,
    "illumination": "ConstantLight RGB(1,1,1)",
    "scene_setup_seconds": scene_seconds,
    "first_render_wall_seconds": first_render_seconds,
    "scene_compilation_reports": scene_reports,
    "compilation_reports_through_first_render": reports,
    "settings": {k: str(v) for k, v in pt._settings.items()},
    "specialization_constants": pt._constants,
    "debug_layers": True,
    "rhi_validation": True,
    "features": [str(f) for f in device.features],
    "compiler_options": {
        k: str(getattr(device.slang_session.desc.compiler_options, k))
        for k in [
            "optimization",
            "floating_point_mode",
            "debug_info",
            "matrix_layout",
            "shader_model",
        ]
    },
    "device_info": {
        k: getattr(device.info, k)
        for k in ["api_name", "adapter_name", "timestamp_frequency", "optix_version"]
    },
    "module_cache": False,
    "shader_cache": False,
}
result["source_sha256"] = hashlib.sha256(source.encode()).hexdigest()
result.update(timing)
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
print(
    json.dumps(
        {
            k: result[k]
            for k in [
                "backend",
                "median_gpu_ms",
                "p10_gpu_ms",
                "p90_gpu_ms",
                "clock_summary",
            ]
        },
        indent=2,
    ),
    flush=True,
)
record_identity(args.output.with_suffix(".identity.json"), result["backend"])
