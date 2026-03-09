# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
SlangPy integration test: Permuto hash grid encoding + MLP image fitting baseline.

Matches tiny-cuda-nn's Permuto + FullyFusedMLP test for direct comparison.
Architecture: (x,y) -> PermutoEncoder(8 levels, 2 feat/level) -> MLP 16->128->128->128->3 (Sigmoid) -> RGB

Tests both the PermutoEncoder and FFLayer from neural.slang via slangpy:
  - Feature table trained via atomic gradient accumulation (PointerAddress atomicAdd)
  - MLP trained via DifferentialPtrPair<PointerAddress>
  - End-to-end differentiable encoding + MLP pipeline
"""

from __future__ import annotations

from pathlib import Path

import numpy as np
import pytest

import slangpy as spy

from conftest import get_slangpy_paths, REF_IMAGE_PATH

TEST_DIR = Path(__file__).resolve().parent

MAX_LEVELS = 8
FEAT_PER_LEVEL = 2
HASHMAP_SIZE = 1 << 16  # 65536
FEATURE_TABLE_SIZE = MAX_LEVELS * HASHMAP_SIZE * FEAT_PER_LEVEL  # 1,048,576


def xavier_init(layers):
    """Xavier uniform initialization."""
    params = []
    for inp, out in layers:
        scale = np.sqrt(6.0 / (inp + out))
        params.extend([np.random.uniform(-scale, scale, out * inp), np.zeros(out)])
    return np.concatenate(params).astype("float32")


class Network(spy.InstanceList):
    """Permuto encoder + MLP network."""

    def __init__(self, module, device):
        super().__init__(module["Network"])
        np.random.seed(42)

        # MLP params (16->128->128->128->3)
        mlp_np = xavier_init([(16, 128), (128, 128), (128, 128), (128, 3)])
        self._params = spy.Tensor.from_numpy(device, mlp_np)
        self._params_grad = spy.Tensor.zeros_like(self._params)
        self._mlp_m = spy.Tensor.zeros_like(self._params)
        self._mlp_v = spy.Tensor.zeros_like(self._params)
        self.params = self._params.storage.device_address
        self.params_grad = self._params_grad.storage.device_address

        # Feature table (8 levels x 65536 entries x 2 features)
        ft_np = np.random.uniform(-1e-4, 1e-4, (FEATURE_TABLE_SIZE,)).astype("float32")
        self._feat_table = spy.Tensor.from_numpy(device, ft_np)
        self._feat_grad = spy.Tensor.zeros_like(self._feat_table)
        self._feat_m = spy.Tensor.zeros_like(self._feat_table)
        self._feat_v = spy.Tensor.zeros_like(self._feat_table)
        self.feature_table = self._feat_table.storage.device_address
        self.feature_table_grad = self._feat_grad.storage.device_address

    def optimize(self, module, mlp_lr: float, feat_lr: float, it: int):
        module.optimizer_step(self._params, self._params_grad, self._mlp_m, self._mlp_v, mlp_lr, it)
        module.optimizer_step(self._feat_table, self._feat_grad, self._feat_m, self._feat_v, feat_lr, it)


def _create_device_and_module(device_type=spy.DeviceType.cuda):
    """Create device and load the Permuto+MLP Slang module."""
    include_paths = get_slangpy_paths()

    try:
        device = spy.Device(
            type=device_type,
            compiler_options=spy.SlangCompilerOptions({
                "include_paths": include_paths,
            }),
        )
    except Exception as exc:
        pytest.skip(f"Device not available ({device_type}): {exc}")

    module = spy.Module.load_from_file(device, str(TEST_DIR / "neural_permuto_mlp.slang"))
    return device, module


# ============================================================================
# Tests
# ============================================================================

@pytest.mark.parametrize("device_type", [spy.DeviceType.cuda])
def test_training_convergence_permuto(device_type: spy.DeviceType) -> None:
    """Train Permuto+MLP on GPU for 10K iterations and verify loss convergence."""
    device, module = _create_device_and_module(device_type)
    try:
        network = Network(module, device)
        mlp_p = int(module.get_mlp_params())
        ft_p = int(module.get_feature_table_size())
        print(f"\nMLP params: {mlp_p}, Feature table: {ft_p}, Total: {mlp_p + ft_p}")

        image = spy.Tensor.load_from_image(device, str(REF_IMAGE_PATH), linearize=True)
        res = spy.int2(256, 256)
        batch_size = (64, 64)
        mlp_lr = 0.001
        feat_lr = 0.01
        num_iters = 10000

        loss_out = spy.Tensor.empty_like(image)
        module.show_loss(pixel=spy.call_id(), resolution=res,
                         reference=image, network=network, _result=loss_out)
        initial_loss = float(np.mean(loss_out.to_numpy()))
        print(f"Initial loss: {initial_loss:.6f}")

        for it in range(num_iters):
            module.calculate_grads(
                seed=spy.wang_hash(seed=it, warmup=2),
                batch_index=spy.grid(batch_size),
                batch_size=spy.int2(batch_size),
                reference=image,
                network=network,
            )
            network.optimize(module, mlp_lr, feat_lr, it + 1)

            if (it + 1) % 2000 == 0:
                module.show_loss(pixel=spy.call_id(), resolution=res,
                                 reference=image, network=network, _result=loss_out)
                mid_loss = float(np.mean(loss_out.to_numpy()))
                print(f"  Iter {it+1}: loss = {mid_loss:.6f}")

        module.show_loss(pixel=spy.call_id(), resolution=res,
                         reference=image, network=network, _result=loss_out)
        final_loss = float(np.mean(loss_out.to_numpy()))
        print(f"Final loss: {final_loss:.6f} (initial: {initial_loss:.6f})")

        assert np.isfinite(final_loss), "Final loss is not finite"
        assert final_loss < initial_loss * 0.05, \
            f"Training did not converge: {final_loss:.6f} >= 5% of initial {initial_loss:.6f}"

    finally:
        device.close()


@pytest.mark.parametrize("device_type", [spy.DeviceType.cuda])
def test_inference_quality_permuto(device_type: spy.DeviceType) -> None:
    """Train Permuto+MLP, render full image, and verify reconstruction quality."""
    device, module = _create_device_and_module(device_type)
    try:
        network = Network(module, device)
        image = spy.Tensor.load_from_image(device, str(REF_IMAGE_PATH), linearize=True)
        res = spy.int2(256, 256)
        batch_size = (64, 64)
        mlp_lr = 0.001
        feat_lr = 0.01

        for it in range(10000):
            module.calculate_grads(
                seed=spy.wang_hash(seed=it, warmup=2),
                batch_index=spy.grid(batch_size),
                batch_size=spy.int2(batch_size),
                reference=image,
                network=network,
            )
            network.optimize(module, mlp_lr, feat_lr, it + 1)

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
        pass_05 = np.mean(max_channel_err < 0.05) * 100

        mse = float(np.mean(per_pixel_err ** 2))
        psnr = 10.0 * np.log10(1.0 / mse) if mse > 0 else float("inf")

        print("\nInference quality (256x256, Permuto+MLP):")
        print(f"  MSE:  {mse:.6f}")
        print(f"  PSNR: {psnr:.2f} dB")
        print(f"  Mean error: {mean_err:.4f}")
        print(f"  Error < 0.05: {pass_05:.1f}%")
        print(f"  Error < 0.20: {pass_20:.1f}%")
        print(f"  Error < 0.30: {pass_30:.1f}%")

        # Permuto+MLP should achieve high quality
        assert pass_30 >= 80.0, f"Too few pixels within 0.30: {pass_30:.1f}%"
        assert psnr >= 20.0, f"PSNR too low: {psnr:.2f} dB"

        # Determinism check
        output2 = spy.Tensor.empty_like(image)
        module.render(pixel=spy.call_id(), resolution=res, network=network, _result=output2)
        assert np.allclose(output_np, output2.to_numpy(), atol=1e-5), "Inference not deterministic"
        print("  Determinism: PASS")

        try:
            from PIL import Image as PILImage
            out_img = (np.clip(output_np, 0, 1) * 255).astype(np.uint8)
            if out_img.ndim == 3 and out_img.shape[2] == 3:
                PILImage.fromarray(out_img).save(str(TEST_DIR / "permuto_mlp_output.png"))
                print(f"  Saved: {TEST_DIR / 'permuto_mlp_output.png'}")
        except (ImportError, OSError) as e:
            print(f"  Image save skipped: {e}")

    finally:
        device.close()


@pytest.mark.parametrize("device_type", [spy.DeviceType.cuda])
def test_parameter_count_permuto(device_type: spy.DeviceType) -> None:
    """Verify parameter counts for Permuto encoder + MLP."""
    device, module = _create_device_and_module(device_type)
    try:
        mlp_p = int(module.get_mlp_params())
        ft_p = int(module.get_feature_table_size())

        # MLP: Layer0(16*128+128) + Layer1(128*128+128) + Layer2(same) + Layer3(128*3+3)
        expected_mlp = (16 * 128 + 128) + (128 * 128 + 128) * 2 + (128 * 3 + 3)  # 35587
        expected_ft = FEATURE_TABLE_SIZE  # 2,097,152

        assert mlp_p == expected_mlp, f"MLP: {mlp_p} != {expected_mlp}"
        assert ft_p == expected_ft, f"Feature table: {ft_p} != {expected_ft}"
        print(f"\nParams: MLP={mlp_p}, Feature table={ft_p}, Total={mlp_p + ft_p}")
    finally:
        device.close()


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
