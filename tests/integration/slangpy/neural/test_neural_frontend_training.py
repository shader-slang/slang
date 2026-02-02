# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
SlangPy integration test for neural module FFLayer (Option 2 design).

Tests training convergence for a simple quadratic regression task using:
- FFLayer with storage passed as parameter to eval<S>()
- Manual gradient computation (analytic gradients)
- Simple SGD optimization

We fit a quadratic polynomial y = 2*x^2 - 0.5*x + 0.25 and verify convergence.
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

    # Use pre-built neural module from slang (not compiled from source)
    # The neural module is built as part of slang-neural-module target
    # Enable experimental features since neural is an experimental module
    compiler_options = spy.SlangCompilerOptions(
        {
            "include_paths": [test_dir, SLANG_PATH],
            "debug_info": spy.SlangDebugInfoLevel.standard,
            "enable_experimental_features": True,
        }
    )

    return spy.Device(
        type=device_type,
        enable_debug_layers=True,
        compiler_options=compiler_options,
        label=f"uncached-slangpy-neural-frontend-{device_type.name}",
    )


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_neural_frontend_training_converges(device_type: spy.DeviceType) -> None:
    """
    Test that training converges for a simple quadratic regression task.

    Uses FFLayer with the "Option 2" neural frontend design, where parameter
    storage is passed explicitly as an argument to eval<S>().
    """
    device = _get_device_with_native_neural(device_type)
    try:
        module = spy.Module(device.load_module("test_neural_frontend_training.slang"))

        param_count = int(module.get_param_count())
        assert param_count == 3

        # Fit: y = 2*x^2 - 0.5*x + 0.25
        sample_count = 256
        xs = np.linspace(-1.0, 1.0, sample_count, dtype=np.float32)
        ys = (2.0 * xs * xs - 0.5 * xs + 0.25).astype(np.float32)

        xs_buf = device.create_buffer(data=xs, usage=spy.BufferUsage.shader_resource)
        ys_buf = device.create_buffer(data=ys, usage=spy.BufferUsage.shader_resource)

        rng = np.random.default_rng(0)
        params_init = (0.01 * rng.standard_normal(size=(param_count,))).astype(np.float32)

        params = device.create_buffer(
            data=params_init,
            usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
        )
        grads = device.create_buffer(
            data=np.zeros((param_count,), dtype=np.float32),
            usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
        )

        initial_loss = float(module.eval_loss(params, xs_buf, ys_buf, sample_count))

        learning_rate = 0.1
        steps = 200
        for _ in range(steps):
            module.train_step(params, grads, xs_buf, ys_buf, sample_count, learning_rate)

        final_loss = float(module.eval_loss(params, xs_buf, ys_buf, sample_count))

        # Convergence: should significantly reduce MSE and reach a small absolute error.
        assert final_loss < initial_loss * 1e-2
        assert final_loss < 1e-3

        # Parameter packing: [w0, w1, bias] for y = w0*x + w1*x^2 + bias
        learned = params.to_numpy().view(np.float32)[:param_count]
        expected = np.array([-0.5, 2.0, 0.25], dtype=np.float32)
        assert np.allclose(learned, expected, rtol=0.1, atol=0.1)

    finally:
        device.close()


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
