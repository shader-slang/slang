# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

from __future__ import annotations

import numpy as np
import pytest

import slangpy as spy

from test_utils import (
    WORKGROUP_SIZE,
    compute_metrics,
    create_buffer,
    create_output_buffer,
    load_compute_kernels,
    load_high_level_module,
    load_reference_image_buffer,
    load_reference_image_tensor,
    make_seed,
    mean_loss_from_buffer,
    save_output_image,
    xavier_init,
    zeros_buffer,
)

TRAIN_ITERS = 5000
BATCH_SIZE = (64, 64)
LEARNING_RATE = 0.0001
VECTOR_TYPES = ("inline", "wave", "wave_half")
MLP_LAYERS = [(2, 128), (128, 128), (128, 128), (128, 3)]
ENTRY_POINTS = [
    "compute_calculate_grads",
    "compute_render",
    "compute_show_loss",
    "compute_optimizer_step",
]


class Network(spy.InstanceList):
    def __init__(self, module, device):
        super().__init__(module["Network"])
        params_np = xavier_init(MLP_LAYERS)
        self._params = spy.Tensor.from_numpy(device, params_np)
        self._params_grad = spy.Tensor.zeros_like(self._params)
        self._m = spy.Tensor.zeros_like(self._params)
        self._v = spy.Tensor.zeros_like(self._params)
        self.params = self._params.storage.device_address
        self.params_grad = self._params_grad.storage.device_address

    def optimize(self, module, lr: float, iteration: int):
        module.optimizer_step(self._params, self._params_grad, self._m, self._v, lr, iteration)


class InlineRunner:
    def __init__(self, device_type):
        self.device, self.module = load_high_level_module(
            "neural_mlp_only.slang",
            device_type=device_type,
        )
        self.network = Network(self.module, self.device)
        self.image = load_reference_image_tensor(self.device)
        self.ref_np = self.image.to_numpy()
        self.resolution = spy.int2(256, 256)
        self.loss_out = spy.Tensor.empty_like(self.image)

    def get_total_params(self) -> int:
        return int(self.module.get_total_params())

    def train_step(self, iteration: int) -> None:
        self.module.calculate_grads(
            seed=spy.wang_hash(seed=iteration, warmup=2),
            batch_index=spy.grid(BATCH_SIZE),
            batch_size=spy.int2(BATCH_SIZE),
            reference=self.image,
            network=self.network,
        )
        self.network.optimize(self.module, LEARNING_RATE, iteration + 1)

    def loss(self) -> float:
        self.module.show_loss(
            pixel=spy.call_id(),
            resolution=self.resolution,
            reference=self.image,
            network=self.network,
            _result=self.loss_out,
        )
        return float(np.mean(self.loss_out.to_numpy()))

    def render(self) -> np.ndarray:
        output = spy.Tensor.empty_like(self.image)
        self.module.render(
            pixel=spy.call_id(),
            resolution=self.resolution,
            network=self.network,
            _result=output,
        )
        return output.to_numpy()

    def close(self) -> None:
        self.device.close()


class WaveRunner:
    def __init__(self, device_type, use_half=False):
        wg = 32 if use_half else WORKGROUP_SIZE
        defines = {
            "USE_WAVE_TANGLED": "1",
            "WORKGROUP_SIZE": str(wg),
        }
        if use_half:
            defines["USE_HALF"] = "1"
        self.wg = wg
        self.loss_scale = 1.0
        self.dtype = "float16" if use_half else "float32"
        entry_points = list(ENTRY_POINTS)
        if use_half:
            entry_points.append("compute_optimizer_step_half")
        self.device, self.kernels, self.module = load_compute_kernels(
            "neural_mlp_only.slang",
            entry_points,
            device_type=device_type,
            defines=defines,
        )
        params_np_f32 = xavier_init(MLP_LAYERS, dtype="float32")
        self.param_count = len(params_np_f32)
        params_np = params_np_f32.astype(self.dtype)
        self.params = create_buffer(self.device, params_np)
        self.params_grad = zeros_buffer(self.device, self.param_count, dtype=self.dtype)
        self.use_half = use_half
        if use_half:
            self.params_master = create_buffer(self.device, params_np_f32)
        self.adam_m = zeros_buffer(self.device, self.param_count, dtype="float32")
        self.adam_v = zeros_buffer(self.device, self.param_count, dtype="float32")
        self.ref_buf, self.resolution, self.ref_np = load_reference_image_buffer(self.device)
        self.loss_buf = create_output_buffer(self.device, self.resolution)

    def get_total_params(self) -> int:
        return int(self.module.get_total_params())

    def _ceildiv(self, x: int, y: int) -> int:
        return (x + y - 1) // y

    def train_step(self, iteration: int) -> None:
        batch_count = BATCH_SIZE[0] * BATCH_SIZE[1]
        num_groups = self._ceildiv(batch_count, self.wg)
        self.kernels["compute_calculate_grads"].dispatch(
            thread_count=[num_groups * self.wg, 1, 1],
            seed=make_seed(iteration),
            batch_size=[BATCH_SIZE[0], BATCH_SIZE[1]],
            img_resolution=[self.resolution[0], self.resolution[1]],
            loss_scale=self.loss_scale,
            ref_image=self.ref_buf,
            params=self.params,
            params_grad=self.params_grad,
        )

        param_groups = self._ceildiv(self.param_count, self.wg)
        if self.use_half:
            self.kernels["compute_optimizer_step_half"].dispatch(
                thread_count=[param_groups * self.wg, 1, 1],
                primal=self.params,
                grad=self.params_grad,
                primal_master=self.params_master,
                mean_buf=self.adam_m,
                variance_buf=self.adam_v,
                lr=LEARNING_RATE,
                iter=iteration + 1,
                loss_scale=self.loss_scale,
                param_count=self.param_count,
            )
        else:
            self.kernels["compute_optimizer_step"].dispatch(
                thread_count=[param_groups * self.wg, 1, 1],
                primal=self.params,
                grad=self.params_grad,
                mean_buf=self.adam_m,
                variance_buf=self.adam_v,
                lr=LEARNING_RATE,
                iter=iteration + 1,
                loss_scale=self.loss_scale,
                param_count=self.param_count,
            )

    def loss(self) -> float:
        total_pixels = self.resolution[0] * self.resolution[1]
        num_groups = self._ceildiv(total_pixels, self.wg)
        self.kernels["compute_show_loss"].dispatch(
            thread_count=[num_groups * self.wg, 1, 1],
            img_resolution=[self.resolution[0], self.resolution[1]],
            params=self.params,
            ref_image=self.ref_buf,
            output_loss=self.loss_buf,
        )
        return mean_loss_from_buffer(self.loss_buf)

    def render(self) -> np.ndarray:
        total_pixels = self.resolution[0] * self.resolution[1]
        num_groups = self._ceildiv(total_pixels, self.wg)
        output = create_output_buffer(self.device, self.resolution)
        self.kernels["compute_render"].dispatch(
            thread_count=[num_groups * self.wg, 1, 1],
            img_resolution=[self.resolution[0], self.resolution[1]],
            params=self.params,
            output_image=output,
        )
        return output.to_numpy().view(np.float32).reshape(self.resolution[1], self.resolution[0], 3)

    def close(self) -> None:
        self.device.close()


def _create_runner(vector_type: str, device_type):
    if vector_type == "inline":
        return InlineRunner(device_type)
    if vector_type == "wave":
        return WaveRunner(device_type)
    if vector_type == "wave_half":
        return WaveRunner(device_type, use_half=True)
    raise ValueError(f"Unknown vector type: {vector_type}")


@pytest.mark.parametrize("device_type", [spy.DeviceType.cuda])
@pytest.mark.parametrize("vector_type", VECTOR_TYPES)
def test_training_convergence_mlp_only(device_type: spy.DeviceType, vector_type: str) -> None:
    runner = _create_runner(vector_type, device_type)
    try:
        total_p = runner.get_total_params()
        print(f"\nMLP-only [{vector_type}] params: {total_p}")

        initial_loss = runner.loss()
        print(f"Initial loss: {initial_loss:.6f}")

        for iteration in range(TRAIN_ITERS):
            runner.train_step(iteration)
            if (iteration + 1) % 1000 == 0:
                mid_loss = runner.loss()
                print(f"  Iter {iteration + 1}: loss = {mid_loss:.6f}")

        final_loss = runner.loss()
        print(f"Final loss: {final_loss:.6f} (initial: {initial_loss:.6f})")

        assert np.isfinite(final_loss), "Final loss is not finite"
        assert final_loss < initial_loss * 0.25, (
            f"Training did not converge: {final_loss:.6f} >= 25% of initial {initial_loss:.6f}"
        )
    finally:
        runner.close()


@pytest.mark.parametrize("device_type", [spy.DeviceType.cuda])
@pytest.mark.parametrize("vector_type", VECTOR_TYPES)
def test_inference_quality_mlp_only(device_type: spy.DeviceType, vector_type: str) -> None:
    runner = _create_runner(vector_type, device_type)
    try:
        for iteration in range(TRAIN_ITERS):
            runner.train_step(iteration)

        output_np = runner.render()
        ref_np = runner.ref_np
        assert np.all(np.isfinite(output_np)), "Non-finite inference outputs"

        metrics = compute_metrics(output_np, ref_np)
        print(f"\nMLP-only inference [{vector_type}] (256x256):")
        print(f"  MSE:  {metrics.mse:.6f}")
        print(f"  PSNR: {metrics.psnr:.2f} dB")
        print(f"  Mean error: {metrics.mean_error:.4f}")
        print(f"  Error < 0.20: {metrics.pass_20:.1f}%")
        print(f"  Error < 0.30: {metrics.pass_30:.1f}%")

        assert metrics.pass_30 >= 10.0, f"Too few pixels within 0.30: {metrics.pass_30:.1f}%"

        output_np_2 = runner.render()
        assert np.allclose(output_np, output_np_2, atol=1e-5), "Inference not deterministic"
        print("  Determinism: PASS")

        try:
            save_output_image(output_np, f"mlp_only_{vector_type}_output.png")
        except (ImportError, OSError) as exc:
            print(f"  Image save skipped: {exc}")
    finally:
        runner.close()


@pytest.mark.parametrize("device_type", [spy.DeviceType.cuda])
@pytest.mark.parametrize("vector_type", VECTOR_TYPES)
def test_parameter_count_mlp_only(device_type: spy.DeviceType, vector_type: str) -> None:
    runner = _create_runner(vector_type, device_type)
    try:
        total_mlp = runner.get_total_params()
        expected_mlp = sum(fi * fo + fo for fi, fo in MLP_LAYERS)  # 33795
        assert total_mlp == expected_mlp, f"MLP: {total_mlp} != {expected_mlp}"
        print(f"\nMLP-only [{vector_type}] params: {total_mlp}")
    finally:
        runner.close()


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
