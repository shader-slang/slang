#!/usr/bin/env python3
"""Compare Slang buildAllTilesFromArray vs Tin2 from_array register dumps."""

import subprocess
import struct
import numpy as np
from pathlib import Path

# Run Tin2 dump
print("=== Running Tin2 from_array dump ===")
result = subprocess.run(["/tmp/test_from_array_dump"], capture_output=True, text=True)
print(result.stdout[:2000])

# Run Slang dump via slangpy
print("\n=== Running Slang buildAllTilesFromArray dump ===")
try:
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
        entry_points=[module.entry_point("dump_build_tiles")]
    )
    kernel = device.create_compute_kernel(program)

    # 32 lanes * 16 regs = 512 uint32s
    output_buf = device.create_buffer(
        data=np.zeros(512, dtype=np.uint32),
        usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
    )

    kernel.dispatch(thread_count=[32, 1, 1], output=output_buf)
    device.wait()

    data = output_buf.to_numpy().view(np.uint32)

    # Print in same format as Tin2
    for tile in range(2):
        print(f"\n--- Slang tile[{tile}] ---")
        for lane in range(8):
            regs_str = ""
            for r in range(4):  # Only regs 0-3 matter for MMA
                val = data[lane * 16 + tile * 8 + r]
                lo = np.frombuffer(struct.pack('<I', val), dtype=np.float16)[0]
                hi = np.frombuffer(struct.pack('<I', val), dtype=np.float16)[1]
                regs_str += f"  r{r}=({float(lo):7.3f},{float(hi):7.3f})"
            print(f"  lane {lane:2d}:{regs_str}")
        print("  ...")

    device.close()

except Exception as e:
    print(f"Error: {e}")
    import traceback
    traceback.print_exc()
