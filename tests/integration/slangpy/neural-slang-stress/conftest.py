# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
Pytest configuration for neural.slang stress tests.

Requirements:
- slangpy installed with slang >= 2026.4 (neural module support)
- CUDA-capable GPU
"""

from __future__ import annotations

import pytest


def pytest_configure(config):
    config.addinivalue_line(
        "markers", "neural_large: mark test as neural large-model stress test"
    )


def pytest_collection_modifyitems(config, items):  # noqa: ARG001
    for item in items:
        item.add_marker(pytest.mark.neural_large)
