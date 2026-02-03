# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
"""
Neural smoke test that actually exercises Slang autodiff (`bwd_diff(...)`).

Important constraints:
- No dependency on sample apps under `samples/`.
- No dependency on external assets (e.g. image files).

This uses the test-local Slang module `fflayer-bug-repro.slang` which imports the
experimental `neural` module and calls `bwd_diff(loss)(DifferentialPtrPair<Storage>(...), ...)`.
"""

from __future__ import annotations

from pathlib import Path

import numpy as np
import pytest

import slangpy as spy
from slangpy.core.calldata import SLANG_PATH
from slangpy.testing import helpers


def _get_device_with_native_neural(device_type: spy.DeviceType) -> spy.Device:
    if helpers.should_skip_test_for_device(device_type):
        pytest.skip(f"Device type {device_type.name} not selected for this test run")

    test_dir = Path(__file__).resolve().parent
    
    # Try to enable experimental features (required for neural module)
    # This option may not be available in older slangpy versions
    compiler_options_dict = {
        "include_paths": [test_dir, SLANG_PATH],
        "debug_info": spy.SlangDebugInfoLevel.standard,
    }
    
    try:
        # Try with experimental features enabled (newer slangpy)
        compiler_options_dict["enable_experimental_features"] = True
        compiler_options = spy.SlangCompilerOptions(compiler_options_dict)
    except (RuntimeError, TypeError):
        # Fall back without experimental features (older slangpy)
        del compiler_options_dict["enable_experimental_features"]
        compiler_options = spy.SlangCompilerOptions(compiler_options_dict)
        pytest.skip("slangpy version does not support enable_experimental_features (required for neural module)")

    return spy.Device(
        type=device_type,
        enable_debug_layers=True,
        compiler_options=compiler_options,
        label=f"uncached-slangpy-neural-bwd-diff-{device_type.name}",
    )


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_neural_bwd_diff_writes_param_grads(device_type: spy.DeviceType) -> None:
    device = _get_device_with_native_neural(device_type)
    try:
        module = spy.Module(device.load_module("fflayer-bug-repro.slang"))

        # 2*2 weights + 2 biases = 6 floats (matches `fflayer-bug-repo.py`)
        params = device.create_buffer(
            data=np.ones((6,), dtype=np.float32),
            usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
        )
        dparams = device.create_buffer(
            data=np.zeros((6,), dtype=np.float32),
            usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
        )

        module.calculate_grad(input=spy.float2(1, 1), params=params, dparams=dparams)

        dparams_np = dparams.to_numpy().view(np.float32)
        assert np.any(dparams_np != 0.0)
    finally:
        device.close()
