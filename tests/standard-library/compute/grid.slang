// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api cuda

import slang.runtime;
import slang.compute.buffer_utils;
import slang.compute.texture_utils;

[host]
void main()
{
    let values = RWStructuredBuffer<uint>::alloc(128);
    values.fill(0u);
    uint scale = 3;
    Grid(67u).dispatch(
        (uint i) =>
        { InterlockedAdd(values[i], (i + 1) * scale); });
    Grid(67u).dispatch(
        (uint i) =>
        { values[i] *= 2; });
    for (uint i = 0; i < 128; ++i)
        assert(readBack(values, i) == (i < 67 ? (i + 1) * 6 : 0));

    values.fill(0u);
    Grid(uint2(11, 5))
        .dispatch<8, 4>(
            (uint2 p) =>
            {
                uint i = p.x + 11 * p.y;
                InterlockedAdd(values[i], i + 1);
            });
    for (uint i = 0; i < 128; ++i)
        assert(readBack(values, i) == (i < 55 ? i + 1 : 0));

    values.fill(0u);
    Grid(uint3(7, 3, 2))
        .dispatch<4, 2, 2>(
            (uint3 p) =>
            {
                uint i = p.x + 7 * (p.y + 3 * p.z);
                InterlockedAdd(values[i], i + 1);
            });
    for (uint i = 0; i < 128; ++i)
        assert(readBack(values, i) == (i < 42 ? i + 1 : 0));

    // Runtime sizes reuse the same callback type; zero extents must not invoke any callbacks.
    values.fill(0u);
    let increment = (uint i) =>
    { InterlockedAdd(values[i], 1u); };
    Grid(3u).dispatch(increment);
    Grid(7u).dispatch(increment);
    Grid(0u).dispatch(increment);
    Grid(uint2(0, 17))
        .dispatch(
            (uint2) =>
            { values[127] = 1; });
    Grid(uint3(17, 1, 0))
        .dispatch(
            (uint3) =>
            { values[127] = 1; });
    Grid(0u).dispatchCooperative<2>(
        (uint, uint, bool) =>
        { values[127] = 1; });
    Grid(uint2(17, 0))
        .dispatchCooperative<1>(
            (uint2, uint, bool) =>
            { values[127] = 1; });
    for (uint i = 0; i < 128; ++i)
        assert(readBack(values, i) == (i < 3 ? 2 : (i < 7 ? 1 : 0)));

    // Both subgroup collectives and cross-subgroup barriers include padding lanes.
    values.fill(0u);
    Grid(100u).dispatchCooperative<2>(
        (uint i, uint localIndex, bool isValid) =>
        {
            static groupshared uint subgroupTotals[2];
            uint active = WaveActiveSum(uint(isValid));
            if (WaveIsFirstLane())
                subgroupTotals[localIndex / 32] = active;
            GroupMemoryBarrierWithGroupSync();
            if (isValid)
                values[i] = subgroupTotals[0] + subgroupTotals[1] + 1000 * localIndex;
        });
    for (uint i = 0; i < 128; ++i)
    {
        uint expected = i < 100 ? (i < 64 ? 64 : 36) + 1000 * (i % 64) : 0;
        assert(readBack(values, i) == expected);
    }

    values.fill(0u);
    Grid(uint3(5, 2, 2))
        .dispatchCooperative<1>(
            (uint3 p, uint localIndex, bool isValid) =>
            {
                uint active = WaveActiveSum(uint(isValid));
                if (isValid)
                    values[p.x + 5 * (p.y + 2 * p.z)] = active + 100 * localIndex;
            });
    for (uint i = 0; i < 20; ++i)
        assert(readBack(values, i) == 5 + 100 * (i % 5));
    assert(readBack(values, 20) == 0);

    // Spill into a second physical group row without allocating a corresponding large buffer.
    values.fill(0u);
    uint count = 65535u * 64u + 3u;
    Grid(count).dispatch(
        (uint i) =>
        {
            if (i % 64 == 0)
                InterlockedAdd(values[0], 1u);
            if (i >= count - 3)
                InterlockedAdd(values[1], 1u);
        });
    assert(readBack(values, 0) == 65536);
    assert(readBack(values, 1) == 3);
    Grid(count).dispatchCooperative<2>(
        (uint i, uint localIndex, bool isValid) =>
        {
            uint active = WaveActiveSum(uint(isValid));
            if (localIndex == 0)
                InterlockedAdd(values[2], 1u);
            if (isValid && i == count - 1)
                values[3] = active;
        });
    // Entire padding groups are skipped uniformly, not counted as logical workgroups.
    assert(readBack(values, 2) == 65536);
    assert(readBack(values, 3) == 3);

    // The fill/zero wrappers use the same spilling path, including the final partial group.
    let large = RWStructuredBuffer<uint>::alloc(count);
    large.fill(7u);
    Grid(count).dispatch(
        (uint i) =>
        {
            if (large[i] != 7)
                InterlockedAdd(values[4], 1u);
        });
    assert(readBack(values, 4) == 0);
    large.zero();
    Grid(count).dispatch(
        (uint i) =>
        {
            if (large[i] != 0)
                InterlockedAdd(values[4], 1u);
        });
    assert(readBack(values, 4) == 0);

    let texture = allocateTexture2D<uint>(17, 3);
    texture.fill(42u);
    Grid(uint2(17, 3))
        .dispatch<8, 8>(
            (uint2 p) =>
            { values[p.x + 17 * p.y] = texture[p]; });
    for (uint i = 0; i < 51; ++i)
        assert(readBack(values, i) == 42);

    // A resource-free closure is also legal; no dummy binding is required by dispatch.
    Grid(1u).dispatch(
        (uint) =>
        {});
    assert(readBack(values, 3) == 42);
}
