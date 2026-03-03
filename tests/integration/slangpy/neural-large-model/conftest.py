# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
Pytest configuration for neural.slang large-model integration tests.

These tests exercise neural.slang on the GPU via slangpy:
- Training convergence (MLP + latent grid)
- Inference correctness
- Forward/backward stress across all activation functions

Requirements:
- slangpy installed with slang >= 2026.1 (neural module support)
- GPU with Vulkan support
"""

from __future__ import annotations

import os
from pathlib import Path

import pytest

# Directory containing this conftest and test .slang files
TEST_DIR = Path(__file__).resolve().parent

# Reference image used by training/inference tests
REF_IMAGE_PATH = Path(__file__).resolve().parent.parent.parent.parent / "neural" / "neural-mlp-test-image.png"


def get_slangpy_paths():
    """Return include paths for slangpy's slang modules and neural standard module."""
    import slangpy as spy
    pkg_dir = Path(spy.__file__).parent

    paths = [str(TEST_DIR)]

    slangpy_slang = pkg_dir / "slang"
    if slangpy_slang.exists():
        paths.append(str(slangpy_slang))

    for d in sorted(pkg_dir.iterdir()):
        if d.is_dir() and d.name.startswith("slang-standard-module-"):
            paths.append(str(d))
            break

    env_neural = os.environ.get("NEURAL_MODULE_PATH")
    if env_neural:
        paths.append(env_neural)

    return paths


def pytest_configure(config):
    config.addinivalue_line(
        "markers", "neural_large: mark test as neural large-model integration test"
    )


def pytest_collection_modifyitems(config, items):  # noqa: ARG001
    for item in items:
        item.add_marker(pytest.mark.neural_large)
