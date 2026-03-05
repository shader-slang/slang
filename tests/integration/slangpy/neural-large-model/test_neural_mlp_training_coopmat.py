# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
SlangPy integration test: neural.slang MLP with WaveTangledVector (CoopMat / tensor core).

Uses cooperative matrix operations for the MLP forward and backward pass.
Key differences from test_neural_mlp_training.py (InlineVector version):
  - WaveTangledVector: 32 threads per warp cooperate on MMA
  - Low-level kernel dispatch with [numthreads(32,1,1)]
  - Kaiming uniform initialization (larger weights for fp16 training)
  - Loss scaling to prevent gradient underflow in half-precision backward pass

Architecture: UV -> bilinear sample 32x32x4 grid -> MLP 4->128->128->128->3 -> RGB
"""

from __future__ import annotations

import math
from pathlib import Path

import numpy as np
import pytest

import slangpy as spy

from conftest import get_slangpy_paths, REF_IMAGE_PATH

TEST_DIR = Path(__file__).resolve().parent

WORKGROUP_SIZE = 64
INPUT_DIM = 4
HIDDEN_DIM = 128
OUTPUT_DIM = 3
LATENT_W = 32
LATENT_H = 32
LOSS_SCALE = 1.0


def kaiming_uniform_init(layers):
    """Kaiming uniform initialization for ReLU/LeakyReLU networks.

    Weights ~ Uniform(-a, +a) where a = sqrt(6 / fan_in).
    Biases initialized to zero.
    """
    params = []
    for fan_in, fan_out in layers:
        a = math.sqrt(6.0 / fan_in)
        params.append(np.random.uniform(-a, a, fan_out * fan_in).astype("float32"))
        params.append(np.zeros(fan_out, dtype="float32"))
    return np.concatenate(params)


def _ceildiv(a, b):
    return (a + b - 1) // b


def _create_device_and_kernels(extra_defines=None):
    """Create CUDA device, load module, link compute kernels."""
    include_paths = get_slangpy_paths()
    defines = {"WORKGROUP_SIZE": str(WORKGROUP_SIZE)}
    if extra_defines:
        defines.update(extra_defines)

    try:
        device = spy.Device(
            type=spy.DeviceType.cuda,
            compiler_options=spy.SlangCompilerOptions({
                "include_paths": include_paths,
                "defines": defines,
            }),
        )
    except Exception as exc:
        pytest.skip(f"CUDA device not available: {exc}")

    raw_module = device.load_module(str(TEST_DIR / "neural_mlp_gpu_coopmat.slang"))

    def _make_kernel(name):
        program = device.link_program(
            modules=[raw_module],
            entry_points=[raw_module.entry_point(name)],
        )
        return device.create_compute_kernel(program)

    kernels = {
        "calculate_grads": _make_kernel("compute_calculate_grads"),
        "render": _make_kernel("compute_render"),
        "show_loss": _make_kernel("compute_show_loss"),
        "optimizer_step": _make_kernel("compute_optimizer_step"),
    }

    high_level_module = spy.Module(raw_module)

    return device, kernels, high_level_module


def _create_buffers(device, mlp_dtype="float32"):
    """Create all GPU buffers. mlp_dtype controls MLP param precision."""
    np.random.seed(42)

    mlp_params_np = kaiming_uniform_init(
        [(INPUT_DIM, HIDDEN_DIM), (HIDDEN_DIM, HIDDEN_DIM),
         (HIDDEN_DIM, HIDDEN_DIM), (HIDDEN_DIM, OUTPUT_DIM)]
    ).astype(mlp_dtype)
    total_mlp = len(mlp_params_np)

    latent_np = np.random.uniform(0.0, 1.0, (LATENT_H, LATENT_W, INPUT_DIM)).astype("float32")
    total_latent = latent_np.size

    usage_rw = spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access

    bufs = {}
    bufs["mlp_params"] = device.create_buffer(data=mlp_params_np, usage=usage_rw)
    bufs["mlp_grads"] = device.create_buffer(
        data=np.zeros(total_mlp, dtype=mlp_dtype), usage=usage_rw)
    bufs["mlp_adam_m"] = device.create_buffer(
        data=np.zeros(total_mlp, dtype=mlp_dtype), usage=usage_rw)
    bufs["mlp_adam_v"] = device.create_buffer(
        data=np.zeros(total_mlp, dtype=mlp_dtype), usage=usage_rw)

    bufs["latent"] = device.create_buffer(data=latent_np.ravel(), usage=usage_rw)
    bufs["latent_grad"] = device.create_buffer(
        data=np.zeros(total_latent, dtype="float32"), usage=usage_rw)
    bufs["latent_adam_m"] = device.create_buffer(
        data=np.zeros(total_latent, dtype="float32"), usage=usage_rw)
    bufs["latent_adam_v"] = device.create_buffer(
        data=np.zeros(total_latent, dtype="float32"), usage=usage_rw)

    bufs["total_mlp"] = total_mlp
    bufs["total_latent"] = total_latent

    return bufs


def _load_reference_image(device):
    """Load the 256x256 reference image as a flat float buffer [H*W*3]."""
    from PIL import Image as PILImage
    img = PILImage.open(str(REF_IMAGE_PATH)).convert("RGB")
    img_np = np.array(img, dtype="float32") / 255.0  # [H, W, 3] in [0,1]
    h, w = img_np.shape[:2]
    usage_rw = spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access
    buf = device.create_buffer(data=img_np.ravel(), usage=usage_rw)
    return buf, w, h, img_np


def _dispatch_grads(kernels, bufs, ref_buf, resolution, batch_size, seed, loss_scale):
    """Dispatch one training step: compute gradients via CoopMat backward pass."""
    batch_count = batch_size[0] * batch_size[1]
    num_groups = _ceildiv(batch_count, WORKGROUP_SIZE)
    kernels["calculate_grads"].dispatch(
        thread_count=[num_groups * WORKGROUP_SIZE, 1, 1],
        seed=seed,
        batch_size=[batch_size[0], batch_size[1]],
        img_resolution=[resolution[0], resolution[1]],
        loss_scale=loss_scale,
        params=bufs["mlp_params"],
        params_grad=bufs["mlp_grads"],
        latent_data=bufs["latent"],
        latent_grad=bufs["latent_grad"],
        ref_image=ref_buf,
        latent_width=LATENT_W,
        latent_height=LATENT_H,
    )


def _dispatch_optimizer(kernels, bufs, lr, iteration, loss_scale):
    """Dispatch Adam optimizer step for MLP params and latent grid."""
    for prefix, count_key in [("mlp", "total_mlp"), ("latent", "total_latent")]:
        param_count = bufs[count_key]
        num_groups = _ceildiv(param_count, WORKGROUP_SIZE)
        p_key = prefix if prefix == "latent" else f"{prefix}_params"
        g_key = f"{prefix}_grad" if prefix == "latent" else f"{prefix}_grads"
        kernels["optimizer_step"].dispatch(
            thread_count=[num_groups * WORKGROUP_SIZE, 1, 1],
            primal=bufs[p_key],
            grad=bufs[g_key],
            mean_buf=bufs[f"{prefix}_adam_m"],
            variance_buf=bufs[f"{prefix}_adam_v"],
            lr=lr,
            iter=iteration,
            loss_scale=loss_scale,
            param_count=param_count,
        )


def _dispatch_show_loss(kernels, bufs, ref_buf, resolution, loss_buf):
    """Dispatch loss evaluation over the full image."""
    total_pixels = resolution[0] * resolution[1]
    num_groups = _ceildiv(total_pixels, WORKGROUP_SIZE)
    kernels["show_loss"].dispatch(
        thread_count=[num_groups * WORKGROUP_SIZE, 1, 1],
        img_resolution=[resolution[0], resolution[1]],
        params=bufs["mlp_params"],
        latent_data=bufs["latent"],
        ref_image=ref_buf,
        output_loss=loss_buf,
        latent_width=LATENT_W,
        latent_height=LATENT_H,
    )


def _dispatch_render(kernels, bufs, resolution, output_buf):
    """Dispatch full-image render (inference)."""
    total_pixels = resolution[0] * resolution[1]
    num_groups = _ceildiv(total_pixels, WORKGROUP_SIZE)
    kernels["render"].dispatch(
        thread_count=[num_groups * WORKGROUP_SIZE, 1, 1],
        img_resolution=[resolution[0], resolution[1]],
        params=bufs["mlp_params"],
        latent_data=bufs["latent"],
        output_image=output_buf,
        latent_width=LATENT_W,
        latent_height=LATENT_H,
    )


# ============================================================================
# Tests
# ============================================================================

@pytest.mark.parametrize("device_type", [spy.DeviceType.cuda])
def test_training_convergence_coopmat(device_type: spy.DeviceType) -> None:
    """Train MLP on GPU using WaveTangledVector for 5K iterations."""
    device, kernels, module = _create_device_and_kernels()
    try:
        bufs = _create_buffers(device)
        ref_buf, w, h, _ = _load_reference_image(device)
        resolution = (w, h)
        batch_size = (64, 64)
        lr = 0.001
        num_iters = 5000

        total_mlp = int(module.get_total_params())
        grid_p = int(module.get_grid_params())
        print(f"\nCoopMat MLP params: {total_mlp}, Grid: {grid_p}, "
              f"Total: {total_mlp + grid_p}, Loss scale: {LOSS_SCALE}")

        total_pixels = w * h
        usage_rw = spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access
        loss_buf = device.create_buffer(
            data=np.zeros(total_pixels * 3, dtype="float32"), usage=usage_rw)

        _dispatch_show_loss(kernels, bufs, ref_buf, resolution, loss_buf)
        initial_loss = float(np.mean(loss_buf.to_numpy().view(np.float32)))
        print(f"Initial loss: {initial_loss:.6f}")

        for it in range(num_iters):
            seed = int((it * 2654435761) & 0xFFFFFFFF)
            _dispatch_grads(kernels, bufs, ref_buf, resolution, batch_size,
                            seed, LOSS_SCALE)
            _dispatch_optimizer(kernels, bufs, lr, it + 1, LOSS_SCALE)

            if (it + 1) % 1000 == 0:
                _dispatch_show_loss(kernels, bufs, ref_buf, resolution, loss_buf)
                mid_loss = float(np.mean(loss_buf.to_numpy().view(np.float32)))
                print(f"  Iter {it+1}: loss = {mid_loss:.6f}")

        _dispatch_show_loss(kernels, bufs, ref_buf, resolution, loss_buf)
        final_loss = float(np.mean(loss_buf.to_numpy().view(np.float32)))
        print(f"Final loss: {final_loss:.6f} (initial: {initial_loss:.6f})")

        assert np.isfinite(final_loss), "Final loss is not finite"
        assert final_loss < initial_loss * 0.15, \
            f"Training did not converge: {final_loss:.6f} >= 15% of initial {initial_loss:.6f}"

    finally:
        device.close()


@pytest.mark.parametrize("device_type", [spy.DeviceType.cuda])
def test_inference_quality_coopmat(device_type: spy.DeviceType) -> None:
    """Train with CoopMat, then render full image and compare against reference."""
    device, kernels, _module = _create_device_and_kernels()
    try:
        bufs = _create_buffers(device)
        ref_buf, w, h, ref_np = _load_reference_image(device)
        resolution = (w, h)
        batch_size = (64, 64)
        lr = 0.001
        total_pixels = w * h
        usage_rw = spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access

        for it in range(5000):
            seed = int((it * 2654435761) & 0xFFFFFFFF)
            _dispatch_grads(kernels, bufs, ref_buf, resolution, batch_size,
                            seed, LOSS_SCALE)
            _dispatch_optimizer(kernels, bufs, lr, it + 1, LOSS_SCALE)

        output_buf = device.create_buffer(
            data=np.zeros(total_pixels * 3, dtype="float32"), usage=usage_rw)
        _dispatch_render(kernels, bufs, resolution, output_buf)

        output_np = output_buf.to_numpy().view(np.float32).reshape(h, w, 3)

        assert np.all(np.isfinite(output_np)), "Non-finite inference outputs"

        per_pixel_err = np.abs(output_np - ref_np)
        mean_err = per_pixel_err.mean()
        max_channel_err = per_pixel_err.reshape(-1, 3).max(axis=1)
        pass_30 = np.mean(max_channel_err < 0.30) * 100
        pass_20 = np.mean(max_channel_err < 0.20) * 100

        print("\nCoopMat inference quality (256x256):")
        print(f"  Mean error: {mean_err:.4f}")
        print(f"  Error < 0.20: {pass_20:.1f}%")
        print(f"  Error < 0.30: {pass_30:.1f}%")

        assert pass_30 >= 60.0, f"Too few pixels within 0.30: {pass_30:.1f}%"

        output2_buf = device.create_buffer(
            data=np.zeros(total_pixels * 3, dtype="float32"), usage=usage_rw)
        _dispatch_render(kernels, bufs, resolution, output2_buf)
        output2_np = output2_buf.to_numpy().view(np.float32).reshape(h, w, 3)
        assert np.allclose(output_np, output2_np, atol=1e-5), "Inference not deterministic"
        print("  Determinism: PASS")

        try:
            from PIL import Image as PILImage
            out_img = (np.clip(output_np, 0, 1) * 255).astype(np.uint8)
            PILImage.fromarray(out_img).save(str(TEST_DIR / "gpu_training_output_coopmat.png"))
            print("  Saved: gpu_training_output_coopmat.png")
        except ImportError:
            print("  PIL not available, skipping image save")
        except OSError as e:
            print(f"  Failed to save image: {e}")

    finally:
        device.close()


@pytest.mark.parametrize("device_type", [spy.DeviceType.cuda])
def test_parameter_count_coopmat(device_type: spy.DeviceType) -> None:
    """FFLayer.ParameterCount must match expected values (same as InlineVector)."""
    device, _kernels, module = _create_device_and_kernels()
    try:
        total_mlp = int(module.get_total_params())
        grid_params = int(module.get_grid_params())

        expected_mlp = 640 + 16512 + 16512 + 387  # = 34051
        expected_grid = LATENT_W * LATENT_H * INPUT_DIM  # = 4096

        assert total_mlp == expected_mlp, f"MLP: {total_mlp} != {expected_mlp}"
        assert grid_params == expected_grid, f"Grid: {grid_params} != {expected_grid}"
        print(f"\nCoopMat params: MLP={total_mlp}, Grid={grid_params}, "
              f"Total={total_mlp + grid_params}")
    finally:
        device.close()


def _train_n_iters(defines, mlp_dtype, loss_scale, num_iters):
    """Train for N iterations and return the loss curve. Used for cross-precision comparison."""
    device, kernels, module = _create_device_and_kernels(extra_defines=defines)
    try:
        bufs = _create_buffers(device, mlp_dtype=mlp_dtype)
        ref_buf, w, h, _ = _load_reference_image(device)
        resolution = (w, h)
        batch_size = (64, 64)
        lr = 0.001
        total_pixels = w * h
        usage_rw = spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access
        loss_buf = device.create_buffer(
            data=np.zeros(total_pixels * 3, dtype="float32"), usage=usage_rw)

        _dispatch_show_loss(kernels, bufs, ref_buf, resolution, loss_buf)
        losses = [float(np.mean(loss_buf.to_numpy().view(np.float32)))]

        for it in range(num_iters):
            seed = int((it * 2654435761) & 0xFFFFFFFF)
            _dispatch_grads(kernels, bufs, ref_buf, resolution, batch_size,
                            seed, loss_scale)
            _dispatch_optimizer(kernels, bufs, lr, it + 1, loss_scale)

            if (it + 1) % 50 == 0:
                _dispatch_show_loss(kernels, bufs, ref_buf, resolution, loss_buf)
                losses.append(float(np.mean(loss_buf.to_numpy().view(np.float32))))

        return losses
    finally:
        device.close()


@pytest.mark.parametrize("device_type", [spy.DeviceType.cuda])
def test_float_vs_half_coopmat(device_type: spy.DeviceType) -> None:
    """Compare float and half WaveTangledVector training to detect CoopMat bugs.

    Exercises the hybrid precision path (MatC size != MatB size) that triggered
    the shared memory reuse bug fixed in PR #10384. If the half path produces
    significantly worse results or NaN, it indicates a CoopMat correctness issue.
    """
    num_iters = 200

    print("\n--- Float WaveTangledVector ---")
    float_losses = _train_n_iters(
        defines=None, mlp_dtype="float32", loss_scale=1.0, num_iters=num_iters)
    print(f"  Initial: {float_losses[0]:.4f}  Final: {float_losses[-1]:.4f}")

    print("--- Half WaveTangledVector ---")
    half_losses = _train_n_iters(
        defines={"USE_HALF": "1"}, mlp_dtype="float16", loss_scale=1.0,
        num_iters=num_iters)
    print(f"  Initial: {half_losses[0]:.4f}  Final: {half_losses[-1]:.4f}")

    print("--- Comparison ---")
    float_reduction = (1 - float_losses[-1] / float_losses[0]) * 100
    half_reduction = (1 - half_losses[-1] / half_losses[0]) * 100
    print(f"  Float: {float_reduction:.0f}% reduction")
    print(f"  Half:  {half_reduction:.0f}% reduction")

    assert np.isfinite(float_losses[-1]), "Float training produced NaN"
    assert np.isfinite(half_losses[-1]), "Half training produced NaN"

    assert float_losses[-1] < float_losses[0] * 0.5, \
        f"Float didn't converge: {float_losses[-1]:.4f} >= 50% of {float_losses[0]:.4f}"
    assert half_losses[-1] < half_losses[0] * 0.5, \
        f"Half didn't converge: {half_losses[-1]:.4f} >= 50% of {half_losses[0]:.4f}"

    ratio = half_losses[-1] / max(float_losses[-1], 1e-10)
    print(f"  Half/Float loss ratio: {ratio:.2f}")
    assert ratio < 5.0, \
        f"Half loss too far from float: ratio {ratio:.2f} (half={half_losses[-1]:.4f}, float={float_losses[-1]:.4f})"
    print("  PASS: both converge, half within 5x of float")


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
