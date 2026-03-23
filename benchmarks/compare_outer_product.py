#!/usr/bin/env python3
"""Compare Slang outerProductAccumulate vs Tin2 outer_product_reduce."""

import subprocess
import numpy as np
from pathlib import Path

# Run Tin2
print("=== Tin2 outer_product_reduce ===")
result = subprocess.run(["/tmp/test_outer_product_dump"], capture_output=True, text=True)
print(result.stdout)

# Run Slang
print("=== Slang outerProductAccumulate ===")
import slangpy as spy
from slangpy.core.calldata import SLANG_PATH

script_dir = Path(__file__).resolve().parent
for parent in [script_dir] + list(script_dir.parents):
    build_dir = parent / "build"
    if build_dir.is_dir():
        for candidate in sorted(build_dir.rglob("neural.slang-module")):
            neural_dir = candidate.parent.parent
            break
        break

defines = {"INPUT_SIZE": "32", "OUTPUT_SIZE": "16", "SUBGROUP_COUNT": "1"}
include_paths = [str(script_dir), str(neural_dir), SLANG_PATH]
compiler_options = spy.SlangCompilerOptions({
    "include_paths": include_paths,
    "defines": defines,
})

device = spy.Device(
    type=spy.DeviceType.cuda,
    compiler_options=compiler_options,
    bindless_options=spy.BindlessDesc(buffer_count=1000),
)

module = device.load_module("benchmark_single_layer_outer_product.slang")
program = device.link_program(
    modules=[module],
    entry_points=[module.entry_point("compute_outer_product")]
)
kernel = device.create_compute_kernel(program)

# The output is in native tiled layout
# For OUTPUT_SIZE=16, INPUT_SIZE=32: NTileRows=1, NTileCols=2
# TILED_WEIGHT_COUNT = 1*2*256 = 512
tiled_weight_count = 512
output_buf = device.create_buffer(
    data=np.zeros(tiled_weight_count, dtype=np.float16),
    usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
)

kernel.dispatch(thread_count=[32, 1, 1], weightGradBuffer=output_buf, batch_size=1)
device.wait()

data = output_buf.to_numpy().view(np.float16)

# Print in similar format to Tin2 (8 halves per lane per sub-tile)
for tile in range(2):
    print(f"--- Slang tile {tile} ---")
    for lane in range(8):
        base = tile * 256 + lane * 8
        vals = data[base:base+8].astype(np.float32)
        print(f"  lane {lane:2d}: " + " ".join(f"{v:8.3f}" for v in vals))
    print("  ...\n")

device.close()
