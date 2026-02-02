# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
Pytest configuration for neural slangpy integration tests.

These tests require:
- slangpy installed (pip install slangpy)
- GPU with Vulkan or CUDA support
"""

from __future__ import annotations

import pytest


def pytest_configure(config):
    """Register custom markers."""
    config.addinivalue_line(
        "markers", "neural: mark test as neural module integration test"
    )


def pytest_collection_modifyitems(config, items):
    """Add neural marker to all tests in this directory."""
    for item in items:
        item.add_marker(pytest.mark.neural)
