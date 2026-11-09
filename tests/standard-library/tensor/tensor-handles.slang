// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12

import slang.tensor;
import slang.resource.resizable_buffer;
import slang.runtime.compute.grid;
import slang.pipeline_rt;
import slang.runtime.core.simple_assert;

/// Uses the regular tensor API after resolving a nested descriptor handle in ray generation.
[host]
void checkRayTensor()
{
    let output = Tensor<float3, 2>(uint2(3u, 2u));
    let handle = output.getDescriptorHandle();
    Grid(uint2(3u, 2u))
        .dispatchRays(
            (uint2 index) =>
            {
                let tensor = *handle;
                uint flat = tensor.flatIndex(index);
                bool valid = all(tensor.shape == uint2(3u, 2u)) && tensor.elementCount() == 6 &&
                             tensor.capacity == 6 && all(tensor.indexFromFlat(flat) == index);
                tensor[index] =
                    valid ? float3(float(flat), float(index.x), float(index.y)) : float3(-1.0f);
            });
    for (uint flat = 0; flat < 6; ++flat)
    {
        float3 expected = float3(float(flat), float(flat % 3), float(flat / 3));
        assert(!any(output.readBack(flat) != expected));
    }
}

/// Returning only the nested handle must keep a helper-local allocation alive.
[host]
[noinline]
Tensor<uint, 3>.Handle makeTensorHandle()
{
    var values = Tensor<uint, 3>(uint3(2u, 3u, 4u));
    values.reserve(32);
    Grid(24u).dispatch(
        (uint index) =>
        { values.buffer[index] = 100u + index; });
    return values.getDescriptorHandle();
}

/// Nested captures preserve ownership and the different layouts of rank-three and rank-four views.
[host]
void checkLifetimeAndRanks()
{
    let source = makeTensorHandle();
    let output = Tensor<uint, 4>(uint4(2u, 1u, 3u, 4u));
    let destination = output.getDescriptorHandle();
    Grid(24u).dispatchRays(
        (uint index) =>
        {
            let input = *source;
            let result = *destination;
            bool valid = input.capacity == 32 && input.elementCount() == 24 &&
                         all(input.shape == uint3(2u, 3u, 4u)) &&
                         all(result.shape == uint4(2u, 1u, 3u, 4u)) &&
                         result.flatIndex(result.indexFromFlat(index)) == index;
            result[result.indexFromFlat(index)] = valid ? input[input.indexFromFlat(index)] : 0;
        });
    for (uint index = 0; index < 24; ++index)
        assert(output.readBack(index) == 100u + index);
}

/// Handles snapshot shape, count and capacity while preserving shared writes until reallocation.
[host]
void checkResizeSnapshots()
{
    var values = Tensor<uint, 2>(uint2(3u, 2u));
    values.reserve(16);
    let original = values.getDescriptorHandle();
    values.resize(uint2(2u, 3u));
    let reshaped = values.getDescriptorHandle();
    values.resize(uint2(5u, 4u));
    let grown = values.getDescriptorHandle();
    let output = Tensor<uint, 1>(uint1(3u));
    let destination = output.getDescriptorHandle();
    Grid(1u).dispatchRays(
        (uint index) =>
        {
            let first = *original;
            let second = *reshaped;
            let third = *grown;
            let checks = *destination;
            first[uint2(1u, 1u)] = 42;
            third[uint2(4u, 3u)] = 77;
            checks[0] = all(first.shape == uint2(3u, 2u)) && first.elementCount() == 6 &&
                                first.capacity == 16
                            ? 1
                            : 0;
            checks[1] = all(second.shape == uint2(2u, 3u)) && second.capacity == 16 &&
                                second.elementCount() == 6 && second[uint2(0u, 2u)] == 42
                            ? 1
                            : 0;
            checks[2] = all(third.shape == uint2(5u, 4u)) && third.capacity == 20 &&
                                third.elementCount() == 20 && third[uint2(4u, 3u)] == 77
                            ? 1
                            : 0;
        });
    for (uint index = 0; index < 3; ++index)
        assert(output.readBack(index) == 1);
    assert(values.readBack(19) == 77);
}

/// Buffer handles preserve the minimum allocation and old allocations after growth.
[host]
void checkBufferAndEmptyTensor()
{
    var buffer = ResizeableBuffer<uint>(0);
    let original = buffer.getDescriptorHandle();
    buffer.reserve(8);
    let grown = buffer.getDescriptorHandle();
    let empty = Tensor<uint, 2>(uint2(0u, 3u));
    let emptyHandle = empty.getDescriptorHandle();
    let output = Tensor<uint, 1>(uint1(1u));
    let destination = output.getDescriptorHandle();
    Grid(1u).dispatchRays(
        (uint index) =>
        {
            let first = *original;
            let second = *grown;
            let tensor = *emptyHandle;
            let checks = *destination;
            first.buffer[0] = 5;
            second.buffer[0] = 9;
            checks[0] = first.capacity == 1 && second.capacity == 8 && first.buffer[0] == 5 &&
                                second.buffer[0] == 9 && tensor.elementCount() == 0 &&
                                tensor.capacity == 1 && all(tensor.shape == uint2(0u, 3u))
                            ? 1
                            : 0;
        });
    assert(output.readBack(0) == 1);
    assert(buffer.buffer.readBack(0) == 9);
}

[host]
void main()
{
    checkRayTensor();
    checkLifetimeAndRanks();
    checkResizeSnapshots();
    checkBufferAndEmptyTensor();
}
