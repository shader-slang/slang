// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api cuda

import slang.runtime.compute.grid;
import slang.compute.buffer_utils;
import slang.compute.texture_utils;
import slang.runtime.core.simple_assert;

[host]
void main()
{
    let values = RWStructuredBuffer<uint>::alloc(256);
    values.zero();
    Grid(uint2(35, 3))
        .dispatchWaves<2>(
            (Wave<uint2, 2> wave) =>
            {
                let x = wave.map((uint2 p) => p.x + 1u);
                let doubled = wave.map(x, (uint value) => value * 2u);
                // Padding must contribute zero, but must not leave the collective callback.
                uint sum = WaveActiveSum(doubled.laneValue);
                wave.dispatch(
                    x,
                    doubled,
                    (uint2 p, uint a, uint b) =>
                    { values[p.x + 35 * p.y] = a + b + sum; });
            });
    for (uint i = 0; i < 105; ++i)
    {
        uint x = i % 35;
        uint sum = x < 32 ? 32u * 33u : 2u * (33u + 34u + 35u);
        assert(readBack(values, i) == 3 * (x + 1) + sum);
    }
    assert(readBack(values, 105) == 0);

    Grid(0u).dispatchWaves<1>(
        (Wave<uint, 1> wave) =>
        {
            wave.dispatch(
                (uint i) =>
                { values[i] = 999; });
        });
    Grid(uint2(17, 0))
        .dispatchWaves<4>(
            (Wave<uint2, 4> wave) =>
            {
                wave.dispatch(
                    (uint2 p) =>
                    { values[p.x] = 999; });
            });
    assert(readBack(values, 0) == 3 + 32 * 33);
    Grid(uint3(3, 2, 2))
        .dispatchWaves<1>(
            (Wave<uint3, 1> wave) =>
            {
                let a = wave.map((uint3 p) => p.x + 10u);
                wave.dispatch(
                    a,
                    (uint3 p, uint value) =>
                    { InterlockedAdd(values[128 + p.x + 3 * (p.y + 2 * p.z)], value); });
            });
    for (uint i = 0; i < 12; ++i)
        assert(readBack(values, 128 + i) == 10 + i % 3);
    assert(readBack(values, 140) == 0);

    // Each map owns its inactive value; collective adapters can request a neutral element
    // without requiring the scalar callback to handle padding or run on invalid coordinates.
    Grid(1u).dispatchWaves<2>(
        (Wave<uint, 2> wave) =>
        {
            let a = wave.map((uint i) => i + 1u, 2u);
            let b = wave.map(a, (uint x) => x * 3u, 5u);
            uint sumA = WaveActiveSum(a.laneValue);
            uint sumB = WaveActiveSum(b.laneValue);
            wave.dispatch(
                (uint i) =>
                {
                    values[200 + i] = sumA;
                    values[201 + i] = sumB;
                });
        });
    assert(readBack(values, 200) == 1 + 31 * 2);
    assert(readBack(values, 201) == 3 + 31 * 5);

    // map preserves a tuple result. Explicit unzip works for more than two components and
    // preserves each component's padding without running another scalar callback.
    Grid(3u).dispatchWaves<1>(
        (Wave<uint, 1> wave) =>
        {
            let prepared =
                wave.map((uint i) => makeTuple(i + 1u, i * 2u, i == 2u), makeTuple(0u, 7u, false));
            let parts = prepared.unzip();
            uint total = WaveActiveSum(parts._0.laneValue);
            uint stateTotal = WaveActiveSum(parts._1.laneValue);
            uint flagTotal = WaveActiveSum(uint(parts._2.laneValue));
            wave.dispatch(
                parts._1,
                (uint i, uint doubled) =>
                {
                    values[210 + i] = total + doubled;
                    values[220 + i] = stateTotal;
                    values[230 + i] = flagTotal;
                });
            // Tuples can also be consumed directly; map never implicitly splits them.
            let sums = wave.map(prepared, (Tuple<uint, uint, bool> value) => value._0 + value._1);
            wave.dispatch(
                sums,
                (uint i, uint sum) =>
                { values[240 + i] = sum; });
        });
    assert(readBack(values, 210) == 6);
    assert(readBack(values, 211) == 8);
    assert(readBack(values, 212) == 10);
    assert(readBack(values, 213) == 0);
    assert(readBack(values, 220) == 6 + 29 * 7);
    assert(readBack(values, 230) == 1);
    assert(readBack(values, 240) == 1);
    assert(readBack(values, 241) == 4);
    assert(readBack(values, 242) == 7);
    assert(readBack(values, 243) == 0);
}
