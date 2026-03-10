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
MLP_LEARNING_RATE = 0.001
FEATURE_LEARNING_RATE = 0.01
VECTOR_TYPES = ("inline", "wave")
MLP_LAYERS = [(16, 128), (128, 128), (128, 128), (128, 3)]
MAX_LEVELS = 8
FEAT_PER_LEVEL = 2
HASHMAP_SIZE = 1 << 16
FEATURE_TABLE_SIZE = MAX_LEVELS * HASHMAP_SIZE * FEAT_PER_LEVEL
ENTRY_POINTS = [
    "compute_calculate_grads",
    "compute_render",
    "compute_show_loss",
    "compute_optimizer_step",
]


class Network(spy.InstanceList):
    def __init__(self, module, device):
        super().__init__(module["Network"])
        mlp_np = xavier_init(MLP_LAYERS)
        feature_rng = np.random.default_rng(42)
        ft_np = feature_rng.uniform(-1e-4, 1e-4, (FEATURE_TABLE_SIZE,)).astype("float32")

        self._params = spy.Tensor.from_numpy(device, mlp_np)
        self._params_grad = spy.Tensor.zeros_like(self._params)
        self._mlp_m = spy.Tensor.zeros_like(self._params)
        self._mlp_v = spy.Tensor.zeros_like(self._params)
        self.params = self._params.storage.device_address
        self.params_grad = self._params_grad.storage.device_address

        self._feat_table = spy.Tensor.from_numpy(device, ft_np)
        self._feat_grad = spy.Tensor.zeros_like(self._feat_table)
        self._feat_m = spy.Tensor.zeros_like(self._feat_table)
        self._feat_v = spy.Tensor.zeros_like(self._feat_table)
        self.feature_table = self._feat_table.storage.device_address
        self.feature_table_grad = self._feat_grad.storage.device_address

    def optimize(self, module, iteration: int):
        module.optimizer_step(
            self._params,
            self._params_grad,
            self._mlp_m,
            self._mlp_v,
            MLP_LEARNING_RATE,
            iteration,
        )
        module.optimizer_step(
            self._feat_table,
            self._feat_grad,
            self._feat_m,
            self._feat_v,
            FEATURE_LEARNING_RATE,
            iteration,
        )


class InlineRunner:
    def __init__(self, device_type):
        self.device, self.module = load_high_level_module(
            "neural_permuto_mlp.slang",
            device_type=device_type,
        )
        self.network = Network(self.module, self.device)
        self.image = load_reference_image_tensor(self.device)
        self.ref_np = self.image.to_numpy()
        self.resolution = spy.int2(256, 256)
        self.loss_out = spy.Tensor.empty_like(self.image)

    def get_param_counts(self):
        return int(self.module.get_mlp_params()), int(self.module.get_feature_table_size())

    def train_step(self, iteration: int) -> None:
        self.module.calculate_grads(
            seed=spy.wang_hash(seed=iteration, warmup=2),
            batch_index=spy.grid(BATCH_SIZE),
            batch_size=spy.int2(BATCH_SIZE),
            reference=self.image,
            network=self.network,
        )
        self.network.optimize(self.module, iteration + 1)

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
    def __init__(self, device_type):
        defines = {
            "USE_WAVE_TANGLED": "1",
            "WORKGROUP_SIZE": str(WORKGROUP_SIZE),
        }
        self.device, self.kernels, self.module = load_compute_kernels(
            "neural_permuto_mlp.slang",
            ENTRY_POINTS,
            device_type=device_type,
            defines=defines,
        )
        mlp_np = xavier_init(MLP_LAYERS)
        feature_rng = np.random.default_rng(42)
        feature_np = feature_rng.uniform(-1e-4, 1e-4, (FEATURE_TABLE_SIZE,)).astype("float32")

        self.mlp_count = len(mlp_np)
        self.feature_count = len(feature_np)
        self.params = create_buffer(self.device, mlp_np)
        self.params_grad = zeros_buffer(self.device, self.mlp_count)
        self.params_m = zeros_buffer(self.device, self.mlp_count)
        self.params_v = zeros_buffer(self.device, self.mlp_count)
        self.feature_table = create_buffer(self.device, feature_np)
        self.feature_grad = zeros_buffer(self.device, self.feature_count)
        self.feature_m = zeros_buffer(self.device, self.feature_count)
        self.feature_v = zeros_buffer(self.device, self.feature_count)
        self.ref_buf, self.resolution, self.ref_np = load_reference_image_buffer(self.device)
        self.loss_buf = create_output_buffer(self.device, self.resolution)

    def _ceildiv(self, x: int, y: int) -> int:
        return (x + y - 1) // y

    def get_param_counts(self):
        return int(self.module.get_mlp_params()), int(self.module.get_feature_table_size())

    def train_step(self, iteration: int) -> None:
        batch_count = BATCH_SIZE[0] * BATCH_SIZE[1]
        num_groups = self._ceildiv(batch_count, WORKGROUP_SIZE)
        self.kernels["compute_calculate_grads"].dispatch(
            thread_count=[num_groups * WORKGROUP_SIZE, 1, 1],
            seed=make_seed(iteration),
            batch_size=[BATCH_SIZE[0], BATCH_SIZE[1]],
            img_resolution=[self.resolution[0], self.resolution[1]],
            ref_image=self.ref_buf,
            params=self.params,
            params_grad=self.params_grad,
            feature_table=self.feature_table,
            feature_table_grad=self.feature_grad,
        )

        for primal, grad, mean_buf, variance_buf, lr, count in [
            (self.params, self.params_grad, self.params_m, self.params_v, MLP_LEARNING_RATE, self.mlp_count),
            (self.feature_table, self.feature_grad, self.feature_m, self.feature_v, FEATURE_LEARNING_RATE, self.feature_count),
        ]:
            num_groups = self._ceildiv(count, WORKGROUP_SIZE)
            self.kernels["compute_optimizer_step"].dispatch(
                thread_count=[num_groups * WORKGROUP_SIZE, 1, 1],
                primal=primal,
                grad=grad,
                mean_buf=mean_buf,
                variance_buf=variance_buf,
                lr=lr,
                iter=iteration + 1,
                param_count=count,
            )

    def loss(self) -> float:
        total_pixels = self.resolution[0] * self.resolution[1]
        num_groups = self._ceildiv(total_pixels, WORKGROUP_SIZE)
        self.kernels["compute_show_loss"].dispatch(
            thread_count=[num_groups * WORKGROUP_SIZE, 1, 1],
            img_resolution=[self.resolution[0], self.resolution[1]],
            params=self.params,
            feature_table=self.feature_table,
            ref_image=self.ref_buf,
            output_loss=self.loss_buf,
        )
        return mean_loss_from_buffer(self.loss_buf)

    def render(self) -> np.ndarray:
        total_pixels = self.resolution[0] * self.resolution[1]
        num_groups = self._ceildiv(total_pixels, WORKGROUP_SIZE)
        output = create_output_buffer(self.device, self.resolution)
        self.kernels["compute_render"].dispatch(
            thread_count=[num_groups * WORKGROUP_SIZE, 1, 1],
            img_resolution=[self.resolution[0], self.resolution[1]],
            params=self.params,
            feature_table=self.feature_table,
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
    raise AssertionError(f"Unknown vector type: {vector_type}")


@pytest.mark.parametrize("device_type", [spy.DeviceType.cuda])
@pytest.mark.parametrize("vector_type", VECTOR_TYPES)
def test_training_convergence_permuto(device_type: spy.DeviceType, vector_type: str) -> None:
    runner = _create_runner(vector_type, device_type)
    try:
        mlp_p, ft_p = runner.get_param_counts()
        print(f"\nPermuto+MLP [{vector_type}] params: MLP={mlp_p}, Feature table={ft_p}")

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
        assert final_loss < initial_loss * 0.10, (
            f"Training did not converge: {final_loss:.6f} >= 10% of initial {initial_loss:.6f}"
        )
    finally:
        runner.close()


@pytest.mark.parametrize("device_type", [spy.DeviceType.cuda])
@pytest.mark.parametrize("vector_type", VECTOR_TYPES)
def test_inference_quality_permuto(device_type: spy.DeviceType, vector_type: str) -> None:
    runner = _create_runner(vector_type, device_type)
    try:
        for iteration in range(TRAIN_ITERS):
            runner.train_step(iteration)

        output_np = runner.render()
        ref_np = runner.ref_np
        assert np.all(np.isfinite(output_np)), "Non-finite inference outputs"

        metrics = compute_metrics(output_np, ref_np)
        print(f"\nPermuto+MLP inference [{vector_type}] (256x256):")
        print(f"  MSE:  {metrics.mse:.6f}")
        print(f"  PSNR: {metrics.psnr:.2f} dB")
        print(f"  Mean error: {metrics.mean_error:.4f}")
        print(f"  Error < 0.05: {metrics.pass_05:.1f}%")
        print(f"  Error < 0.20: {metrics.pass_20:.1f}%")
        print(f"  Error < 0.30: {metrics.pass_30:.1f}%")

        assert metrics.pass_30 >= 80.0, f"Too few pixels within 0.30: {metrics.pass_30:.1f}%"
        assert metrics.psnr >= 20.0, f"PSNR too low: {metrics.psnr:.2f} dB"

        output_np_2 = runner.render()
        assert np.allclose(output_np, output_np_2, atol=1e-5), "Inference not deterministic"
        print("  Determinism: PASS")

        try:
            save_output_image(output_np, f"permuto_mlp_{vector_type}_output.png")
        except (ImportError, OSError) as exc:
            print(f"  Image save skipped: {exc}")
    finally:
        runner.close()


@pytest.mark.parametrize("device_type", [spy.DeviceType.cuda])
@pytest.mark.parametrize("vector_type", VECTOR_TYPES)
def test_parameter_count_permuto(device_type: spy.DeviceType, vector_type: str) -> None:
    runner = _create_runner(vector_type, device_type)
    try:
        mlp_p, ft_p = runner.get_param_counts()
        expected_mlp = sum(fi * fo + fo for fi, fo in MLP_LAYERS)  # 35587
        expected_ft = FEATURE_TABLE_SIZE  # 1,048,576
        assert mlp_p == expected_mlp, f"MLP: {mlp_p} != {expected_mlp}"
        assert ft_p == expected_ft, f"Feature table: {ft_p} != {expected_ft}"
        print(f"\nPermuto+MLP [{vector_type}] params: MLP={mlp_p}, Feature table={ft_p}")
    finally:
        runner.close()


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
