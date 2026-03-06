# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
SlangPy integration test for neural module FFLayer with CoopMat (WaveTangledVector) backend.

This extends test_neural_frontend_training.py to cover the cooperative matrix backend.
CoopMat requires:
- Vulkan with cooperative matrix extension support
- Explicit compute shaders with [numthreads(32, 1, 1)]
- Types defined inside shader functions

Tests training convergence for a simple quadratic regression task using:
- FFLayer with WaveTangledVector backend
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


# CoopMat only supported on Vulkan (and CUDA with SM 7.0+, but we focus on Vulkan here)
COOPMAT_DEVICE_TYPES: list[spy.DeviceType] = [
    x for x in helpers.DEFAULT_DEVICE_TYPES if x in [spy.DeviceType.vulkan]
]


@pytest.mark.parametrize("device_type", COOPMAT_DEVICE_TYPES)
def test_neural_coopmat_frontend_training_converges(device_type: spy.DeviceType) -> None:
    """
    Verify that the FFLayer frontend still converges on a simple quadratic regression task
    when executed through the CoopMat (cooperative matrix) backend instead of the default
    dense backend.

    This mirrors test_neural_frontend_training_converges, but the kernels use
    WaveTangledVector-based cooperative matrix types and therefore require:
    - a Vulkan device with cooperative matrix feature support
    - explicit compute shaders with [numthreads(32, 1, 1)]
    - matrix types declared inside shader entry points to satisfy CoopMat layout rules

    By comparing the learned quadratic coefficients to the analytic solution, this test
    checks that the CoopMat execution path produces numerically consistent training
    behavior with the standard frontend training test.
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
        # Check for cooperative matrix support
        if not device.has_feature(spy.Feature.cooperative_matrix):
            pytest.skip("Cooperative matrix not supported on this device.")

        module = device.load_module("test_neural_coopmat_training.slang")

        # Get param count via simple function (doesn't need CoopMat)
        param_count = int(spy.Module(module).get_param_count())
        assert param_count == 3

        # Create compute kernels for CoopMat operations
        eval_loss_program = device.link_program(
            modules=[module],
            entry_points=[module.entry_point("compute_eval_loss")]
        )
        eval_loss_kernel = device.create_compute_kernel(eval_loss_program)

        train_step_program = device.link_program(
            modules=[module],
            entry_points=[module.entry_point("compute_train_step")]
        )
        train_step_kernel = device.create_compute_kernel(train_step_program)

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
        loss_buf = device.create_buffer(
            data=np.zeros(1, dtype=np.float32),
            usage=spy.BufferUsage.unordered_access,
        )

        # Dispatch with 1 group of 32 threads (warp-sized for CoopMat)
        eval_loss_kernel.dispatch(
            thread_count=[32, 1, 1],
            params=params,
            xs=xs_buf,
            ys=ys_buf,
            loss_out=loss_buf,
            count=sample_count,
        )
        initial_loss = float(loss_buf.to_numpy().view(np.float32)[0])

        learning_rate = 0.1
        steps = 200
        for _ in range(steps):
            train_step_kernel.dispatch(
                thread_count=[32, 1, 1],
                params=params,
                grads=grads,
                xs=xs_buf,
                ys=ys_buf,
                loss_out=loss_buf,
                count=sample_count,
                learningRate=learning_rate,
            )

        eval_loss_kernel.dispatch(
            thread_count=[32, 1, 1],
            params=params,
            xs=xs_buf,
            ys=ys_buf,
            loss_out=loss_buf,
            count=sample_count,
        )
        final_loss = float(loss_buf.to_numpy().view(np.float32)[0])

        # Convergence: should significantly reduce MSE and reach a small absolute error.
        assert final_loss < initial_loss * 1e-2, f"Final loss {final_loss} not < initial*0.01 {initial_loss * 1e-2}"
        assert final_loss < 1e-3, f"Final loss {final_loss} not < 1e-3"

        # Parameter packing: [w0, w1, bias] for y = w0*x + w1*x^2 + bias
        learned = params.to_numpy().view(np.float32)[:param_count]
        expected = np.array([-0.5, 2.0, 0.25], dtype=np.float32)
        assert np.allclose(learned, expected, rtol=0.1, atol=0.1), f"Learned {learned} != expected {expected}"

    finally:
        device.close()


@pytest.mark.parametrize("device_type", COOPMAT_DEVICE_TYPES)
def test_neural_coopmat_fflayer_forward_pass(device_type: spy.DeviceType) -> None:
    """
    Test FFLayer forward pass with WaveTangledVector produces correct output.
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
        if not device.has_feature(spy.Feature.cooperative_matrix):
            pytest.skip("Cooperative matrix not supported on this device.")

        module = device.load_module("test_neural_coopmat_training.slang")

        forward_program = device.link_program(
            modules=[module],
            entry_points=[module.entry_point("compute_forward_pass")]
        )
        forward_kernel = device.create_compute_kernel(forward_program)

        # Set up known weights for verification
        # Layer: 2 inputs -> 1 output with bias
        # y = w0*x0 + w1*x1 + b
        # With w0=1, w1=2, b=0.5: y = 1*1 + 2*2 + 0.5 = 5.5
        params = device.create_buffer(
            data=np.array([1.0, 2.0, 0.5], dtype=np.float32),
            usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
        )
        result = device.create_buffer(
            data=np.zeros(1, dtype=np.float32),
            usage=spy.BufferUsage.unordered_access,
        )

        # Input: [1.0, 2.0]
        # Dispatch with 1 group of 32 threads
        forward_kernel.dispatch(
            thread_count=[32, 1, 1],
            params=params,
            result=result,
            x0=1.0,
            x1=2.0,
        )

        output = result.to_numpy().view(np.float32)[0]
        expected = 1.0 * 1.0 + 2.0 * 2.0 + 0.5  # = 5.5
        assert np.isclose(output, expected, rtol=0.1), f"Output {output} != expected {expected}"

    finally:
        device.close()


@pytest.mark.parametrize("device_type", COOPMAT_DEVICE_TYPES)
def test_neural_coopmat_multiworkgroup_atomicadd(device_type: spy.DeviceType) -> None:
    """
    Test that atomicAdd works correctly with multiple workgroups using CoopMat.

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
        if not device.has_feature(spy.Feature.cooperative_matrix):
            pytest.skip("Cooperative matrix not supported on this device.")

        module = device.load_module("test_neural_coopmat_training.slang")

        clear_program = device.link_program(
            modules=[module],
            entry_points=[module.entry_point("compute_clear_grads")]
        )
        clear_kernel = device.create_compute_kernel(clear_program)

        grad_program = device.link_program(
            modules=[module],
            entry_points=[module.entry_point("compute_grad_multiworkgroup")]
        )
        grad_kernel = device.create_compute_kernel(grad_program)

        param_count = 3

        # Use sample count that requires multiple workgroups
        sample_count = 64  # 64 workgroups (one sample per workgroup)
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

        # Clear gradients
        clear_kernel.dispatch(thread_count=[1, 1, 1], grads=grads)

        # Compute gradients with multiple workgroups using atomicAdd
        # One workgroup per sample, 32 threads per workgroup
        grad_kernel.dispatch(
            thread_count=[sample_count * 32, 1, 1],
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
        preds = xs + xs * xs + 1.0
        targets = 2.0 * xs * xs - 0.5 * xs + 0.25
        errs = preds - targets
        expected_g0 = (2.0 / sample_count) * np.sum(errs * xs)
        expected_g1 = (2.0 / sample_count) * np.sum(errs * xs * xs)
        expected_gb = (2.0 / sample_count) * np.sum(errs)
        expected_grads = np.array([expected_g0, expected_g1, expected_gb], dtype=np.float32)

        # Allow some tolerance for floating-point atomicAdd accumulation
        assert np.allclose(grads_np, expected_grads, rtol=0.05, atol=1e-3), \
            f"Gradients mismatch: got {grads_np}, expected {expected_grads}"

    finally:
        device.close()


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
