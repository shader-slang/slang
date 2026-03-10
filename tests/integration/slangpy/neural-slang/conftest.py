# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
Pytest configuration for neural slangpy integration tests.

These tests require:
- slangpy installed (pip install slangpy)
- GPU with Vulkan or CUDA support
- neural.slang-module available
"""

from __future__ import annotations

import os
from pathlib import Path

import pytest

TEST_DIR = Path(__file__).resolve().parent


def get_slangpy_include_paths():
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


def pytest_configure(config):
    config.addinivalue_line(
        "markers", "neural: mark test as neural module integration test"
    )


def pytest_collection_modifyitems(config, items):  # noqa: ARG001
    for item in items:
        item.add_marker(pytest.mark.neural)
