// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api cuda

import slang.compute.buffer_utils;

import slang.compute.texture_utils;
import slang.runtime.compute.grid;
import slang.runtime.core.simple_assert;
import slang.runtime.resource.simple_texture;

[host]
void main()
{
    // Partial groups, different input/output types, and nested callback resource captures.
    let input = RWStructuredBuffer<uint>::alloc(80);
    let output = RWStructuredBuffer<float>::alloc(80);
    let visited = RWStructuredBuffer<uint>::alloc(80);
    input.fill(2u);
    output.fill(-1.0);
    visited.fill(99u);
    input.mapInPlace(73u, (uint value) => value + 1u);
    input.mapInPlaceIndexed(73u, (uint i, uint value) => value + i);
    input.mapTo(output, 73u, (uint value) => float(value) * 2.0);
    assert(readBack(output, 72) == 150.0);
    input.mapToIndexed(output, 73u, (uint i, uint value) => float(value + i));
    input.mapIndexed(
        73u,
        (uint i, uint value) =>
        { visited[i] = value; });

    input.mapInPlace(0u, (uint) => 0u);
    input.mapInPlaceIndexed(0u, (uint, uint) => 0u);
    input.mapTo(output, 0u, (uint) => 0.0);
    input.mapToIndexed(output, 0u, (uint, uint) => 0.0);
    input.mapIndexed(
        0u,
        (uint, uint) =>
        { visited[0] = 0; });
    for (uint i = 0; i < 80; ++i)
    {
        assert(readBack(input, i) == (i < 73 ? i + 3 : 2));
        assert(readBack(output, i) == (i < 73 ? float(2 * i + 3) : -1.0));
        assert(readBack(visited, i) == (i < 73 ? i + 3 : 99));
    }

    input.mapInPlace((uint value) => value * 2u);
    input.mapInPlaceIndexed((uint i, uint value) => value + i);
    input.mapTo(output, (uint value) => float(value));
    assert(readBack(output, 79) == 83.0);
    input.mapToIndexed(output, (uint i, uint value) => float(value + i));
    input.mapIndexed(
        (uint i, uint value) =>
        { visited[i] = value; });
    assert(readBack(output, 79) == 162.0);
    assert(readBack(visited, 79) == 83);

    // Scalar-layout read-only resources must keep their layout through the nested lambda.
    let scalarSource = RuntimeArray<uint>::alloc(5);
    for (uint i = 0; i < 5; ++i)
        scalarSource[i] = i + 10;
    let scalar = StructuredBuffer<uint, ScalarDataLayout>::alloc(5);
    copy(scalar, 0, scalarSource, 0, 5);
    scalar.mapToIndexed(output, 4u, (uint i, uint value) => float(value + i));
    assert(readBack(output, 3) == 16.0);
    scalar.mapToIndexed(output, (uint i, uint value) => float(value + i));
    scalar.mapIndexed(
        4u,
        (uint i, uint value) =>
        { visited[i] = value; });
    scalar.mapIndexed(
        (uint i, uint value) =>
        { visited[i] = value; });
    scalar.mapToIndexed(output, 0u, (uint, uint) => 0.0);
    scalar.mapIndexed(
        0u,
        (uint, uint) =>
        { visited[0] = 0; });
    assert(readBack(output, 4) == 18.0);
    assert(readBack(visited, 0) == 10);
    assert(readBack(visited, 4) == 14);

    let texture = allocateTexture2D<float>(17, 9);
    let mapped = allocateTexture2D<float>(17, 9);
    texture.fill(2.0);
    mapped.fill(-1.0);
    texture.mapInPlace(uint2(13, 7), (float value) => value + 1.0);
    texture.mapInPlaceIndexed(
        uint2(13, 7),
        (uint2 p, float value) => value + float(p.x + 17 * p.y));
    texture.mapTo(mapped, uint2(13, 7), (float value) => value * 2.0);
    texture.mapToIndexed(mapped, uint2(13, 7), (uint2 p, float value) => value + float(p.x));
    texture.mapInPlace(uint2(0, 9), (float) => 0.0);
    texture.mapInPlaceIndexed(uint2(17, 0), (uint2, float) => 0.0);
    texture.mapTo(mapped, uint2(0, 9), (float) => 0.0);
    texture.mapToIndexed(mapped, uint2(17, 0), (uint2, float) => 0.0);

    let texels = RWStructuredBuffer<float>::alloc(17 * 9);
    Grid(uint2(17, 9))
        .dispatch<8, 8>(
            (uint2 p) =>
            { texels[p.x + 17 * p.y] = mapped[p]; });
    for (uint y = 0; y < 9; ++y)
        for (uint x = 0; x < 17; ++x)
            assert(
                readBack(texels, x + 17 * y) ==
                (x < 13 && y < 7 ? float(3 + 2 * x + 17 * y) : -1.0));

    texture.fill(1.0);
    texture.mapInPlace((float value) => value * 2.0);
    texture.mapInPlaceIndexed((uint2 p, float value) => value + float(p.x));
    texture.mapTo(mapped, (float value) => value * 3.0);
    mapped.mapToIndexed(texture, (uint2 p, float value) => value + float(p.y));
    Grid(uint2(17, 9))
        .dispatch<8, 8>(
            (uint2 p) =>
            { texels[p.x + 17 * p.y] = texture[p]; });
    assert(readBack(texels, 152) == 62.0);
}
