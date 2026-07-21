# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

from __future__ import annotations

import os
import tempfile
from dataclasses import dataclass
from pathlib import Path

import numpy as np
import pytest

import slangpy as spy

TEST_DIR = Path(__file__).resolve().parent
REF_IMAGE_PATH = TEST_DIR / "neural-mlp-test-image.png"
ARTIFACT_DIR = Path(tempfile.gettempdir()) / "slang-neural-large-model"


def get_slangpy_include_paths():
    """Return include paths for slangpy's slang modules and neural standard module."""
    if spy.__file__ is not None:
        pkg_dir = Path(spy.__file__).parent
    else:
        pkg_dir = Path(spy.__path__[0]) / "slangpy"

    paths = [str(TEST_DIR)]

    slangpy_slang = pkg_dir / "slang"
    if slangpy_slang.exists():
        paths.append(str(slangpy_slang))

    for d in sorted(pkg_dir.iterdir()):
        if d.is_dir() and d.name.startswith("slang-standard-module-"):
            paths.append(str(d))
            inner_slang = d / "slang"
            if inner_slang.is_dir():
                paths.append(str(inner_slang))
            break

    env_neural = os.environ.get("NEURAL_MODULE_PATH")
    if env_neural:
        env_path = Path(env_neural)
        if env_path.is_dir():
            paths.append(str(env_path))
            for d in sorted(env_path.iterdir()):
                if d.is_dir() and d.name.startswith("slang-standard-module-"):
                    paths.append(str(d))
                    inner_slang = d / "slang"
                    if inner_slang.is_dir():
                        paths.append(str(inner_slang))
                    break

    return paths


def get_artifact_path(filename: str) -> Path:
    """Return a temp-dir path for optional locally saved test artifacts."""
    ARTIFACT_DIR.mkdir(parents=True, exist_ok=True)
    return ARTIFACT_DIR / filename

WORKGROUP_SIZE = 64
KNUTH_HASH = 2654435761
USAGE_RW = spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access


@dataclass
class ImageMetrics:
    mse: float
    psnr: float
    mean_error: float
    pass_05: float
    pass_20: float
    pass_30: float


def create_device(device_type=spy.DeviceType.cuda, defines: dict[str, str] | None = None):
    include_paths = get_slangpy_include_paths()
    all_defines = {"UNIT_TEST": "1"}
    if defines:
        all_defines.update(defines)

    compiler_options: dict[str, object] = {
        "include_paths": include_paths,
        "defines": all_defines,
        "enable_experimental_features": True,
    }

    try:
        return spy.Device(
            type=device_type,
            compiler_options=spy.SlangCompilerOptions(compiler_options),
        )
    except Exception as exc:
        pytest.skip(f"Device not available ({device_type}): {exc}")


def load_high_level_module(shader_name: str, device_type=spy.DeviceType.cuda,
                           defines: dict[str, str] | None = None):
    device = create_device(device_type=device_type, defines=defines)
    module = spy.Module.load_from_file(device, str(TEST_DIR / shader_name))
    return device, module


def load_compute_kernels(shader_name: str, entry_points: list[str],
                         device_type=spy.DeviceType.cuda,
                         defines: dict[str, str] | None = None):
    device = create_device(device_type=device_type, defines=defines)
    raw_module = device.load_module(str(TEST_DIR / shader_name))
    kernels = {}
    for name in entry_points:
        program = device.link_program(
            modules=[raw_module],
            entry_points=[raw_module.entry_point(name)],
        )
        kernels[name] = device.create_compute_kernel(program)
    return device, kernels, spy.Module(raw_module)


def xavier_init(layers, seed: int = 42, dtype: str = "float32") -> np.ndarray:
    rng = np.random.default_rng(seed)
    params = []
    for fan_in, fan_out in layers:
        scale = np.sqrt(6.0 / (fan_in + fan_out))
        params.append(rng.uniform(-scale, scale, fan_out * fan_in))
        params.append(np.zeros(fan_out))
    return np.concatenate(params).astype(dtype)


def create_buffer(device, data):
    return device.create_buffer(data=data, usage=USAGE_RW)


def zeros_buffer(device, count: int, dtype: str = "float32"):
    return create_buffer(device, np.zeros(count, dtype=dtype))


def load_reference_image_tensor(device):
    return spy.Tensor.load_from_image(device, str(REF_IMAGE_PATH), linearize=True)


def load_reference_image_numpy() -> np.ndarray:
    from PIL import Image as PILImage

    img = PILImage.open(str(REF_IMAGE_PATH)).convert("RGB")
    return np.array(img, dtype="float32") / 255.0


def load_reference_image_buffer(device):
    image_np = load_reference_image_numpy()
    height, width = image_np.shape[:2]
    return create_buffer(device, image_np.ravel()), (width, height), image_np


def create_output_buffer(device, resolution):
    width, height = resolution
    return create_buffer(device, np.zeros(width * height * 3, dtype="float32"))


def mean_loss_from_buffer(buffer) -> float:
    return float(np.mean(buffer.to_numpy().view(np.float32)))


def make_seed(iteration: int) -> int:
    return int((iteration * KNUTH_HASH) & 0xFFFFFFFF)


def compute_metrics(output_np: np.ndarray, ref_np: np.ndarray) -> ImageMetrics:
    per_pixel_err = np.abs(output_np.astype(np.float32) - ref_np.astype(np.float32))
    max_channel_err = per_pixel_err.reshape(-1, 3).max(axis=1)
    mse = float(np.mean(per_pixel_err ** 2))
    psnr = 10.0 * np.log10(1.0 / mse) if mse > 0 else float("inf")
    return ImageMetrics(
        mse=mse,
        psnr=psnr,
        mean_error=float(per_pixel_err.mean()),
        pass_05=float(np.mean(max_channel_err < 0.05) * 100),
        pass_20=float(np.mean(max_channel_err < 0.20) * 100),
        pass_30=float(np.mean(max_channel_err < 0.30) * 100),
    )


def save_output_image(output_np: np.ndarray, filename: str) -> None:
    from PIL import Image as PILImage

    save_path = get_artifact_path(filename)
    out_img = (np.clip(output_np, 0, 1) * 255).astype(np.uint8)
    PILImage.fromarray(out_img).save(str(save_path))
    print(f"  Saved: {save_path}")
