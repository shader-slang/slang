# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
SlangPy integration test: neural.slang MLP stress test on GPU.

Follows PR #42 Python pattern:
  - spy.Module.load_from_file() for module loading
  - spy.InstanceList for Network struct
  - spy.grid(batch_size) for parallel gradient computation
  - spy.Tensor for all buffer data
  - module.optimizer_step() element-wise Adam
  - module.render() for inference via spy.call_id()

Architecture: UV -> bilinear sample 32x32x4 grid -> MLP 4->128->128->128->3 -> RGB
"""

from __future__ import annotations

from pathlib import Path

import numpy as np
import pytest

import slangpy as spy

from conftest import get_slangpy_paths, REF_IMAGE_PATH

TEST_DIR = Path(__file__).resolve().parent


def xavier_init(layers):
    """Xavier initialization matching PR #42."""
    params = []
    for inp, out in layers:
        scale = np.sqrt(6.0 / (inp + out))
        params.extend([np.random.uniform(-scale, scale, out * inp), np.zeros(out)])
    return np.concatenate(params).astype("float32")


class LatentTexture(spy.InstanceList):
    """Learnable latent grid — matches PR #42's LatentTexture."""

    def __init__(self, module, device, width: int, height: int, num_latents: int):
        super().__init__(module["LatentTexture"])
        initial = np.random.uniform(0.0, 1.0, (height, width, num_latents)).astype("float32")
        self._tex = spy.Tensor.from_numpy(device, initial)
        self._tex_grad = spy.Tensor.zeros_like(self._tex)
        self.texture = self._tex
        self.texture_grads = self._tex_grad
        self._m = spy.Tensor.zeros_like(self._tex)
        self._v = spy.Tensor.zeros_like(self._tex)

    def optimize(self, module, lr: float, it: int):
        module.optimizer_step(self._tex, self._tex_grad, self._m, self._v, lr, it)


class Network(spy.InstanceList):
    """MLP network — matches PR #42's Network."""

    def __init__(self, module, device):
        super().__init__(module["Network"])
        np.random.seed(42)
        params_np = xavier_init([(4, 128), (128, 128), (128, 128), (128, 3)])
        self._params = spy.Tensor.from_numpy(device, params_np)
        self._params_grad = spy.Tensor.zeros_like(self._params)
        self._m = spy.Tensor.zeros_like(self._params)
        self._v = spy.Tensor.zeros_like(self._params)
        self.params = self._params.storage.device_address
        self.params_grad = self._params_grad.storage.device_address
        self.latent_texture = LatentTexture(module, device, 32, 32, 4)

    def optimize(self, module, lr: float, it: int):
        module.optimizer_step(self._params, self._params_grad, self._m, self._v, lr, it)
        self.latent_texture.optimize(module, lr, it)


def _create_device_and_module():
    """Create device and load the Slang module with neural.slang support."""
    include_paths = get_slangpy_paths()

    try:
        device = spy.Device(
            type=spy.DeviceType.cuda,
            compiler_options=spy.SlangCompilerOptions({
                "include_paths": include_paths,
            }),
        )
    except Exception as exc:
        pytest.skip(f"CUDA device not available: {exc}")

    module = spy.Module.load_from_file(device, str(TEST_DIR / "neural_mlp_gpu.slang"))
    return device, module


# ============================================================================
# Tests
# ============================================================================

@pytest.mark.parametrize("device_type", [spy.DeviceType.cuda])
def test_training_convergence(device_type: spy.DeviceType) -> None:
    """Train MLP on GPU for 5K iterations and verify loss convergence."""
    device, module = _create_device_and_module()
    try:
        network = Network(module, device)
        total_p = int(module.get_total_params())
        grid_p = int(module.get_grid_params())
        print(f"\nMLP params: {total_p}, Grid params: {grid_p}, Total: {total_p + grid_p}")

        image = spy.Tensor.load_from_image(device, str(REF_IMAGE_PATH), linearize=True)
        res = spy.int2(256, 256)
        batch_size = (64, 64)
        lr = 0.001
        num_iters = 5000

        # Measure initial loss
        loss_out = spy.Tensor.empty_like(image)
        module.show_loss(pixel=spy.call_id(), resolution=res,
                         reference=image, network=network, _result=loss_out)
        initial_loss = float(np.mean(loss_out.to_numpy()))
        print(f"Initial loss: {initial_loss:.6f}")

        # Training loop — same structure as PR #42
        for it in range(num_iters):
            module.calculate_grads(
                seed=spy.wang_hash(seed=it, warmup=2),
                batch_index=spy.grid(batch_size),
                batch_size=spy.int2(batch_size),
                reference=image,
                network=network,
            )
            network.optimize(module, lr, it + 1)

            if (it + 1) % 1000 == 0:
                module.show_loss(pixel=spy.call_id(), resolution=res,
                                 reference=image, network=network, _result=loss_out)
                mid_loss = float(np.mean(loss_out.to_numpy()))
                print(f"  Iter {it+1}: loss = {mid_loss:.6f}")

        # Final loss
        module.show_loss(pixel=spy.call_id(), resolution=res,
                         reference=image, network=network, _result=loss_out)
        final_loss = float(np.mean(loss_out.to_numpy()))
        print(f"Final loss: {final_loss:.6f} (initial: {initial_loss:.6f})")

        assert np.isfinite(final_loss), "Final loss is not finite"
        assert final_loss < initial_loss * 0.15, \
            f"Training did not converge: {final_loss:.6f} >= 15% of initial {initial_loss:.6f}"

    finally:
        device.close()


@pytest.mark.parametrize("device_type", [spy.DeviceType.cuda])
def test_inference_quality(device_type: spy.DeviceType) -> None:
    """Train, then render full image and compare against reference."""
    device, module = _create_device_and_module()
    try:
        network = Network(module, device)
        image = spy.Tensor.load_from_image(device, str(REF_IMAGE_PATH), linearize=True)
        res = spy.int2(256, 256)
        batch_size = (64, 64)
        lr = 0.001

        # Train
        for it in range(5000):
            module.calculate_grads(
                seed=spy.wang_hash(seed=it, warmup=2),
                batch_index=spy.grid(batch_size),
                batch_size=spy.int2(batch_size),
                reference=image,
                network=network,
            )
            network.optimize(module, lr, it + 1)

        # Render full image via inference
        output = spy.Tensor.empty_like(image)
        module.render(pixel=spy.call_id(), resolution=res, network=network, _result=output)

        output_np = output.to_numpy()
        ref_np = image.to_numpy()

        assert np.all(np.isfinite(output_np)), "Non-finite inference outputs"

        per_pixel_err = np.abs(output_np.astype(np.float32) - ref_np.astype(np.float32))
        mean_err = per_pixel_err.mean()
        max_channel_err = per_pixel_err.reshape(-1, 3).max(axis=1)
        pass_30 = np.mean(max_channel_err < 0.30) * 100
        pass_20 = np.mean(max_channel_err < 0.20) * 100

        print("\nInference quality (256x256 full image):")
        print(f"  Mean error: {mean_err:.4f}")
        print(f"  Error < 0.20: {pass_20:.1f}%")
        print(f"  Error < 0.30: {pass_30:.1f}%")

        assert pass_30 >= 60.0, f"Too few pixels within 0.30: {pass_30:.1f}%"

        # Determinism: render again and compare
        output2 = spy.Tensor.empty_like(image)
        module.render(pixel=spy.call_id(), resolution=res, network=network, _result=output2)
        output2_np = output2.to_numpy()
        assert np.allclose(output_np, output2_np, atol=1e-5), "Inference not deterministic"
        print("  Determinism: PASS")

        # Save output for visual inspection
        save_dir = TEST_DIR
        try:
            from PIL import Image as PILImage
            out_img = (np.clip(output_np, 0, 1) * 255).astype(np.uint8)
            if out_img.ndim == 3 and out_img.shape[2] == 3:
                PILImage.fromarray(out_img).save(str(save_dir / "gpu_training_output.png"))
                print(f"  Saved: {save_dir / 'gpu_training_output.png'}")
        except ImportError:
            print("  PIL not available, skipping image save")
        except OSError as e:
            print(f"  Failed to save image: {e}")

    finally:
        device.close()


@pytest.mark.parametrize("device_type", [spy.DeviceType.cuda])
def test_parameter_count(device_type: spy.DeviceType) -> None:
    """FFLayer.ParameterCount matches expected values."""
    device, module = _create_device_and_module()
    try:
        total_mlp = int(module.get_total_params())
        grid_params = int(module.get_grid_params())

        # Layer0: 4*128+128=640, Layer1: 128*128+128=16512, Layer2: same, Layer3: 128*3+3=387
        expected_mlp = 640 + 16512 + 16512 + 387  # = 34051
        expected_grid = 32 * 32 * 4  # = 4096

        assert total_mlp == expected_mlp, f"MLP: {total_mlp} != {expected_mlp}"
        assert grid_params == expected_grid, f"Grid: {grid_params} != {expected_grid}"
        print(f"\nParams: MLP={total_mlp}, Grid={grid_params}, Total={total_mlp + grid_params}")
    finally:
        device.close()


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
