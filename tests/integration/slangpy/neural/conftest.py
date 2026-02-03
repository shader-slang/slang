# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
Pytest configuration for neural slangpy integration tests.

These tests require:
- slangpy installed (pip install slangpy)
- GPU with Vulkan or CUDA support
"""

from __future__ import annotations

from pathlib import Path

import pytest


def find_neural_module_dir() -> Path | None:
    """Find the neural module directory from the slang build.
    
    Returns the path to slang-standard-module-* directory containing neural.slang-module,
    or None if not found.
    """
    # Look for neural module relative to this conftest file (in slang repo)
    slang_repo = Path(__file__).resolve().parent.parent.parent.parent.parent
    
    # Check common build directories
    for build_type in ["Debug", "Release"]:
        lib_dir = slang_repo / "build" / build_type / "lib"
        if lib_dir.exists():
            # Find slang-standard-module-* directory
            for d in lib_dir.iterdir():
                if d.is_dir() and d.name.startswith("slang-standard-module-"):
                    neural_module = d / "neural.slang-module"
                    if neural_module.exists():
                        return d
    return None


def pytest_configure(config):
    """Register custom markers."""
    config.addinivalue_line(
        "markers", "neural: mark test as neural module integration test"
    )


def pytest_collection_modifyitems(config, items):
    """Add neural marker to all tests in this directory."""
    for item in items:
        item.add_marker(pytest.mark.neural)
