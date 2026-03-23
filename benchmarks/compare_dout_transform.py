#!/usr/bin/env python3
"""Compare dOut after transpose+ChangeMajor between Slang and Tin2."""

import subprocess
import struct
import numpy as np
from pathlib import Path

# Run Tin2 (Stage 3 = change_major_axis(dout.transpose()))
print("=== Tin2: change_major_axis(dout.transpose()) ===")
result = subprocess.run(["/tmp/test_transform_dump"], capture_output=True, text=True)
lines = result.stdout.strip().split('\n')
stage3_start = next(i for i, l in enumerate(lines) if 'Stage 3' in l)
for l in lines[stage3_start:]:
    print(l)

print("\n=== Slang: transposeTilesA + ChangeMajor ===")
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
    entry_points=[module.entry_point("dump_dout_transformed")]
)
kernel = device.create_compute_kernel(program)

# 32 lanes * 8 regs = 256 uint32s
output_buf = device.create_buffer(
    data=np.zeros(256, dtype=np.uint32),
    usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
)

kernel.dispatch(thread_count=[32, 1, 1], output=output_buf)
device.wait()

data = output_buf.to_numpy().view(np.uint32)

for tile in range(2):
    print(f"  sub-tile {tile}:")
    for lane in range(4):
        regs_str = ""
        for r in range(4):
            val = data[lane * 8 + tile * 4 + r]
            lo = np.frombuffer(struct.pack('<I', val), dtype=np.float16)[0]
            hi = np.frombuffer(struct.pack('<I', val), dtype=np.float16)[1]
            regs_str += f" {float(lo):7.3f} {float(hi):7.3f}"
        print(f"    lane {lane:2d}:{regs_str}")
    print("    ...")

device.close()
