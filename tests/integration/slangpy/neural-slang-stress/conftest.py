# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
Pytest configuration for neural.slang large-model integration tests.

These tests exercise neural.slang on the GPU via slangpy:
- MLP-only integration coverage
- Permuto encoder + MLP stress coverage
- InlineVector and WaveTangledVector execution paths

Requirements:
- slangpy installed with slang >= 2026.4 (neural module support)
- CUDA-capable GPU
"""

from __future__ import annotations

import os
from pathlib import Path
import tempfile

import pytest

# Directory containing this conftest and test .slang files
TEST_DIR = Path(__file__).resolve().parent

# Reference image used by training/inference tests
REF_IMAGE_PATH = TEST_DIR / "neural-mlp-test-image.png"

# Optional output images from local runs go under the system temp dir so they
# do not clutter the repo checkout.
ARTIFACT_DIR = Path(tempfile.gettempdir()) / "slang-neural-large-model"


def get_slangpy_paths():
    """Return include paths for slangpy's slang modules and neural standard module."""
    import slangpy as spy

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
        paths.append(env_neural)

    return paths


def get_artifact_path(filename: str) -> Path:
    """Return a temp-dir path for optional locally saved test artifacts."""
    ARTIFACT_DIR.mkdir(parents=True, exist_ok=True)
    return ARTIFACT_DIR / filename


def pytest_configure(config):
    config.addinivalue_line(
        "markers", "neural_large: mark test as neural large-model integration test"
    )


def pytest_collection_modifyitems(config, items):  # noqa: ARG001
    for item in items:
        item.add_marker(pytest.mark.neural_large)
