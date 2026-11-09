// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// TEST:INTERPRET(hgvm): -api d3d12
// TEST:INTERPRET(hgvm): -api vulkan

import slang.runtime.compute.grid;
import slang.runtime.core.simple_assert;

[host]
uint hostHelper(uint value)
{
    return value + 1;
}

interface IGeometry
{
    [host]
    uint build(uint amount);
}

struct Geometry<T> : IGeometry
{
    uint value;

    [host]
    uint build(uint amount) { return hostHelper(value + amount); }
}

struct Object<G : IGeometry>
{
    G geometry;

    [host]
    void run(RWStructuredBuffer<uint> output)
    {
        // The closure's type retains G and its conformance, but its code does not use build.
        Grid(1u).dispatch(
            (uint index) =>
            { output[index] = 42; });
    }
}

[host]
void main()
{
    let output = RWStructuredBuffer<uint>::alloc(1);
    let object = Object<Geometry<uint>>(Geometry<uint>(5));
    assert(object.geometry.build(36) == 42);
    object.run(output);
    assert(output.readBack(0) == 42);
}
