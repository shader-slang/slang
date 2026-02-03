# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

"""
Neural integration tests for bindless resource types.

Reviewer-requested coverage:
- Bindless "pointer type" (raw pointer parameters passed via Buffer.device_address)
- Bindless DescriptorHandle resources (StructuredBuffer<T>.Handle / RWStructuredBuffer<T>.Handle)
"""

from __future__ import annotations

from pathlib import Path

import numpy as np
import pytest

import slangpy as spy
from slangpy.core.calldata import SLANG_PATH
from slangpy.testing import helpers

from conftest import find_neural_module_dir


def _get_device_with_native_neural(device_type: spy.DeviceType) -> spy.Device:
    if helpers.should_skip_test_for_device(device_type):
        pytest.skip(f"Device type {device_type.name} not selected for this test run")

    test_dir = Path(__file__).resolve().parent
    
    # Find neural module directory from slang build
    neural_module_dir = find_neural_module_dir()
    
    include_paths = [test_dir, SLANG_PATH]
    if neural_module_dir:
        include_paths.append(neural_module_dir)

    # Try to enable experimental features (required for neural module)
    # This option may not be available in older slangpy versions
    compiler_options_dict = {
        "include_paths": include_paths,
        "debug_info": spy.SlangDebugInfoLevel.standard,
    }
    
    try:
        # Try with experimental features enabled (newer slangpy)
        compiler_options_dict["enable_experimental_features"] = True
        compiler_options = spy.SlangCompilerOptions(compiler_options_dict)
    except (RuntimeError, TypeError):
        # Fall back without experimental features (older slangpy)
        # Note: This will likely fail when loading neural module
        del compiler_options_dict["enable_experimental_features"]
        compiler_options = spy.SlangCompilerOptions(compiler_options_dict)
        pytest.skip("slangpy version does not support enable_experimental_features (required for neural module)")

    return spy.Device(
        type=device_type,
        enable_debug_layers=True,
        compiler_options=compiler_options,
        label=f"uncached-slangpy-neural-bindless-{device_type.name}",
    )


# Pointer-style bindless params are supported on Vulkan. Keep this test on Vulkan only
# to avoid backend-specific CUDA toolchain requirements for this integration test.
POINTER_DEVICE_TYPES: list[spy.DeviceType] = [
    x for x in helpers.DEFAULT_DEVICE_TYPES if x in [spy.DeviceType.vulkan]
]


@pytest.mark.parametrize("device_type", POINTER_DEVICE_TYPES)
def test_neural_bindless_pointer_type(device_type: spy.DeviceType) -> None:
    device = _get_device_with_native_neural(device_type)
    try:
        module = spy.Module(device.load_module("test_neural_bindless_pointer.slang"))

        buf = device.create_buffer(
            size=4,
            usage=spy.BufferUsage.shader_resource,
            data=np.array([42], dtype=np.int32),
        )

        res = int(module.read_int_ptr(buf.device_address))
        assert res == 42
    finally:
        device.close()


@pytest.mark.parametrize("device_type", helpers.DEFAULT_DEVICE_TYPES)
def test_neural_bindless_descriptor_handle_type(device_type: spy.DeviceType) -> None:
    if device_type == spy.DeviceType.cuda:
        pytest.skip("Bindless DescriptorHandle resources not supported with CUDA yet.")

    device = _get_device_with_native_neural(device_type)
    try:
        if not device.has_feature(spy.Feature.bindless):
            pytest.skip("Bindless not supported on this device.")

        module = device.load_module("test_neural_bindless_descriptor_handle.slang")
        program = device.link_program(
            modules=[module], entry_points=[module.entry_point("compute_main")]
        )
        kernel = device.create_compute_kernel(program)

        buffer_count = 6

        ro_buffers: list[spy.Buffer] = []
        rw_buffers: list[spy.Buffer] = []
        for i in range(buffer_count):
            ro_buffers.append(
                device.create_buffer(
                    size=4 * 4,
                    usage=spy.BufferUsage.shader_resource,
                    data=np.array([i * 10, i * 10 + 1, i * 10 + 2, i * 10 + 3], dtype=np.float32),
                )
            )
            rw_buffers.append(
                device.create_buffer(
                    size=4 * 4,
                    usage=spy.BufferUsage.shader_resource | spy.BufferUsage.unordered_access,
                    data=np.zeros(4, dtype=np.float32),
                )
            )

        buffer_info_layout = module.layout.get_type_layout(
            module.layout.find_type_by_name("StructuredBuffer<BufferInfo>")
        ).element_type_layout

        buffer_infos_buffer = device.create_buffer(
            size=buffer_count * buffer_info_layout.stride,
            usage=spy.BufferUsage.shader_resource,
        )
        results_buffer = device.create_buffer(
            size=buffer_count * 4,
            usage=spy.BufferUsage.unordered_access,
        )

        c = spy.BufferCursor(buffer_info_layout, buffer_infos_buffer, load_before_write=False)
        for i in range(buffer_count):
            c[i].ro_buffer = ro_buffers[i].descriptor_handle_ro
            c[i].rw_buffer = rw_buffers[i].descriptor_handle_rw
            c[i].offset = i % 4
        c.apply()

        kernel.dispatch(
            thread_count=[buffer_count, 1, 1],
            buffer_infos=buffer_infos_buffer,
            results=results_buffer,
        )

        results = results_buffer.to_numpy().view(np.float32)
        expected_results = np.array(
            [
                0,  # buffer 0, offset 0
                11,  # buffer 1, offset 1
                22,  # buffer 2, offset 2
                33,  # buffer 3, offset 3
                40,  # buffer 4, offset 0
                51,  # buffer 5, offset 1
            ],
            dtype=np.float32,
        )
        assert np.allclose(results, expected_results)

        # Verify RW buffers were written.
        for i in range(buffer_count):
            rw_data = rw_buffers[i].to_numpy().view(np.float32)
            offset = i % 4
            expected_value = (i * 10 + offset) + 100.0
            assert np.isclose(rw_data[offset], expected_value)
    finally:
        device.close()


if __name__ == "__main__":
    pytest.main([__file__, "-v", "-s"])
