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

# Directory containing test .slang files
TEST_DIR = Path(__file__).resolve().parent


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_neural_frontend_training_converges(device_type: spy.DeviceType) -> None:
    """
    Test that training converges for a simple quadratic regression task.

    Uses FFLayer with the "Option 2" neural frontend design, where parameter
    storage is passed explicitly as an argument to eval<S>().
    """
    if helpers.should_skip_test_for_device(device_type):
        pytest.skip(f"Device type {device_type.name} not selected for this test run")

    device = spy.Device(
        type=device_type,
        enable_debug_layers=True,
        compiler_options=spy.SlangCompilerOptions({
            "include_paths": [TEST_DIR, SLANG_PATH],
            "enable_experimental_features": True,
        }),
    )
    try:
        module = spy.Module(device.load_module("test_neural_inline_vector_training.slang"))

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


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_neural_multiworkgroup_atomicadd(device_type: spy.DeviceType) -> None:
    """
    Test that atomicAdd works correctly with multiple workgroups.

    This test verifies that gradient accumulation via atomicAdd produces
    correct results when dispatched across multiple workgroups.
    """
    if helpers.should_skip_test_for_device(device_type):
        pytest.skip(f"Device type {device_type.name} not selected for this test run")

    device = spy.Device(
        type=device_type,
        enable_debug_layers=True,
        compiler_options=spy.SlangCompilerOptions({
            "include_paths": [TEST_DIR, SLANG_PATH],
            "enable_experimental_features": True,
        }),
    )
    try:
        module = device.load_module("test_neural_inline_vector_training.slang")

        # Create compute kernel from entry point
        grad_program = device.link_program(
            modules=[module],
            entry_points=[module.entry_point("compute_grad_multiworkgroup")]
        )
        grad_kernel = device.create_compute_kernel(grad_program)

        param_count = 3

        # Use sample count that requires multiple workgroups (> 32 threads)
        sample_count = 256  # 8 workgroups of 32 threads
        xs = np.linspace(-1.0, 1.0, sample_count, dtype=np.float32)
        ys = (2.0 * xs * xs - 0.5 * xs + 0.25).astype(np.float32)

        xs_buf = device.create_buffer(data=xs, usage=spy.BufferUsage.shader_resource)
        ys_buf = device.create_buffer(data=ys, usage=spy.BufferUsage.shader_resource)

        # Known parameters for gradient verification
        params = device.create_buffer(
            data=np.array([1.0, 1.0, 1.0], dtype=np.float32),
            usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
        )
        grads = device.create_buffer(
            data=np.zeros((param_count,), dtype=np.float32),
            usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
        )

        # Compute gradients with multiple workgroups using atomicAdd
        num_workgroups = (sample_count + 31) // 32
        grad_kernel.dispatch(
            thread_count=[num_workgroups * 32, 1, 1],
            params=params,
            grads=grads,
            xs=xs_buf,
            ys=ys_buf,
            count=sample_count,
        )

        # Verify gradients are computed (non-zero)
        grads_np = grads.to_numpy().view(np.float32)[:param_count]
        assert np.any(grads_np != 0.0), "Gradients should be non-zero"

        # Compute expected gradients analytically
        # For y = w0*x + w1*x^2 + b with params [1, 1, 1]:
        # pred = x + x^2 + 1
        # target = 2*x^2 - 0.5*x + 0.25
        # err = pred - target = x + x^2 + 1 - 2*x^2 + 0.5*x - 0.25 = 1.5*x - x^2 + 0.75
        # dL/dw0 = (2/N) * sum(err * x)
        # dL/dw1 = (2/N) * sum(err * x^2)
        # dL/db  = (2/N) * sum(err)
        preds = xs + xs * xs + 1.0
        targets = 2.0 * xs * xs - 0.5 * xs + 0.25
        errs = preds - targets
        expected_g0 = (2.0 / sample_count) * np.sum(errs * xs)
        expected_g1 = (2.0 / sample_count) * np.sum(errs * xs * xs)
        expected_gb = (2.0 / sample_count) * np.sum(errs)
        expected_grads = np.array([expected_g0, expected_g1, expected_gb], dtype=np.float32)

        # Allow some tolerance for floating-point atomicAdd accumulation
        assert np.allclose(grads_np, expected_grads, rtol=0.01, atol=1e-4), \
            f"Gradients mismatch: got {grads_np}, expected {expected_grads}"

    finally:
        device.close()


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
