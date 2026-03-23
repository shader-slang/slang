#!/usr/bin/env python3
"""Dump 3 stages of Slang dOut transformation."""
import struct, numpy as np
from pathlib import Path
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
compiler_options = spy.SlangCompilerOptions({
    "include_paths": [str(script_dir), str(neural_dir), SLANG_PATH],
    "defines": defines,
})
device = spy.Device(type=spy.DeviceType.cuda, compiler_options=compiler_options,
                    bindless_options=spy.BindlessDesc(buffer_count=1000))
module = device.load_module("benchmark_single_layer_outer_product.slang")
program = device.link_program(modules=[module],
    entry_points=[module.entry_point("dump_dout_transformed")])
kernel = device.create_compute_kernel(program)

output_buf = device.create_buffer(data=np.zeros(768, dtype=np.uint32),
    usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access)
kernel.dispatch(thread_count=[32, 1, 1], output=output_buf)
device.wait()
data = output_buf.to_numpy().view(np.uint32)

stages = ["Original", "After reg swap", "After ChangeMajor"]
for s in range(3):
    print(f"=== {stages[s]} ===")
    for tile in range(2):
        print(f"  tile {tile}:")
        for lane in range(4):
            vals = ""
            for r in range(4):
                val = data[s * 256 + lane * 8 + tile * 4 + r]
                lo = np.frombuffer(struct.pack('<I', val), dtype=np.float16)[0]
                hi = np.frombuffer(struct.pack('<I', val), dtype=np.float16)[1]
                vals += f"  r{r}=({float(lo):7.3f},{float(hi):7.3f})"
            print(f"    lane {lane}:{vals}")
        print("    ...")
    print()
device.close()
