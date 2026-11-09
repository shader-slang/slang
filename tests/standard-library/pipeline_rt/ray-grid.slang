// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api d3d12

import slang.runtime.compute.grid;
import slang.pipeline_rt;
import slang.runtime.core.simple_assert;

[host]
void main()
{
    let output = RWStructuredBuffer<uint>::alloc(12);
    let handle = output.getDescriptorHandle();
    Grid(uint2(3, 2))
        .dispatchRays(
            (uint2 index) =>
            { handle[index.y * 3 + index.x] = index.x + index.y * 10; });
    Grid(uint3(3, 1, 2))
        .dispatchRays(
            (uint3 index) =>
            { handle[6 + index.z * 3 + index.x] = index.x + index.z * 100; });
    // Empty grids must not execute the callback or overwrite the preceding results.
    Grid(uint(0)).dispatchRays(
        (uint index) =>
        { handle[0] = 999; });
    Grid(uint3(1, 0, 1))
        .dispatchRays(
            (uint3 index) =>
            { handle[0] = 999; });
    for (uint row = 0; row < 2; ++row)
    {
        for (uint x = 0; x < 3; ++x)
        {
            assert(output.readBack(row * 3 + x) == x + row * 10);
            assert(output.readBack(6 + row * 3 + x) == x + row * 100);
        }
    }
}
