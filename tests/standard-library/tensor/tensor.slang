// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api cuda

import slang.runtime.compute.grid;
import slang.tensor;
import slang.runtime.core.simple_assert;

[host]
void main()
{
    let line = Tensor<uint, 1>(uint1(5));
    assert(line.elementCount() == 5);
    assert(line.flatIndex(uint1(4)) == 4);
    Grid(5u).dispatch(
        (uint i) =>
        { line[uint1(i)] = 10 + i; });
    for (uint i = 0; i < 5; ++i)
        assert(readBack(line.buffer, i) == 10 + i);

    // Nonsquare dimensions expose transposed/last-axis-contiguous indexing.
    var image = Tensor<uint, 2>(uint2(7, 3));
    assert(image.elementCount() == 21);
    assert(image.flatIndex(uint2(2, 1)) == 9);
    Grid(image.shape)
        .dispatch(
            (uint2 p) =>
            { image[p] = p.x + 100 * p.y; });
    for (uint y = 0; y < 3; ++y)
        for (uint x = 0; x < 7; ++x)
            assert(readBack(image.buffer, x + 7 * y) == x + 100 * y);

    // Shrink then regrow without allocating. The saved tensor must still alias the storage.
    let original = image;
    image.resize(uint2(2, 2));
    assert(image.capacity == 21 && image.elementCount() == 4);
    assert(image.flatIndex(uint2(1, 1)) == 3);
    image.resize(uint2(5, 4));
    assert(image.capacity == 21 && image.elementCount() == 20);
    Grid(image.shape)
        .dispatch(
            (uint2 p) =>
            { image[p] = 200 + p.x + 5 * p.y; });
    assert(readBack(original.buffer, 19) == 219);
    assert(readBack(original.buffer, 20) == 206);
    assert(!any(original.shape != uint2(7, 3)));

    // Reserve is capacity-only. Growth leaves earlier captures on their original allocation.
    image.reserve(40);
    assert(image.capacity == 40 && !any(image.shape != uint2(5, 4)));
    assert(image.elementCount() == 20);
    image.resize(uint2(9, 4));
    Grid(image.shape)
        .dispatch(
            (uint2 p) =>
            { image[p] = 500 + p.x + 9 * p.y; });
    assert(readBack(image.buffer, 35) == 535);
    assert(readBack(original.buffer, 19) == 219);
    image.reserve(1);
    assert(image.capacity == 40);
    image.resize(uint2(11, 5));
    assert(image.capacity == 55 && image.elementCount() == 55);

    // Match the viewer's nested ordinary/wave callbacks, including non-scalar elements and
    // bool storage. Padding lanes must not access tensors.
    let inputs = Tensor<float2, 2>(image.shape);
    let valid = Tensor<bool, 2>(image.shape);
    Grid(image.shape)
        .dispatch(
            (uint2 p) =>
            {
                inputs[p] = float2(p);
                valid[p] = p.x % 2 == 0;
            });
    Grid(image.shape)
        .dispatchWaves<2>(
            (Wave<uint2, 2> wave) =>
            {
                let values = wave.map((uint2 p) => inputs[p]);
                wave.dispatch(
                    values,
                    (uint2 p, float2 value) =>
                    { image[p] = valid[p] ? uint(value.x + 100 * value.y) : 999; });
            });
    for (uint y = 0; y < 5; ++y)
        for (uint x = 0; x < 11; ++x)
            assert(readBack(image.buffer, x + 11 * y) == (x % 2 == 0 ? x + 100 * y : 999));

    let volume = Tensor<uint, 3>(uint3(3, 4, 2));
    assert(volume.elementCount() == 24);
    assert(volume.flatIndex(uint3(1, 2, 1)) == 19);
    Grid(volume.shape)
        .dispatch(
            (uint3 p) =>
            { volume[p] = p.x + 10 * p.y + 100 * p.z; });
    for (uint z = 0; z < 2; ++z)
        for (uint y = 0; y < 4; ++y)
            for (uint x = 0; x < 3; ++x)
                assert(readBack(volume.buffer, x + 3 * (y + 4 * z)) == x + 10 * y + 100 * z);

    let batch = Tensor<uint, 4>(uint4(2, 3, 4, 5));
    assert(batch.elementCount() == 120);
    assert(batch.flatIndex(uint4(1, 2, 3, 4)) == 119);
    Grid(uint3(2, 3, 4))
        .dispatch(
            (uint3 p) =>
            {
                for (uint w = 0; w < 5; ++w)
                    batch[uint4(p, w)] = p.x + 10 * p.y + 100 * p.z + 1000 * w;
            });
    for (uint w = 0; w < 5; ++w)
        for (uint z = 0; z < 4; ++z)
            for (uint y = 0; y < 3; ++y)
                for (uint x = 0; x < 2; ++x)
                    assert(
                        readBack(batch.buffer, x + 2 * (y + 3 * (z + 4 * w))) ==
                        x + 10 * y + 100 * z + 1000 * w);

    // Empty shapes are valid, keep capacity, and must not overflow on preceding dimensions.
    image.resize(uint2(0, 5));
    assert(image.elementCount() == 0 && image.capacity == 55);
    Grid(image.shape)
        .dispatch(
            (uint2 p) =>
            { image[p] = 12345; });
    assert(readBack(image.buffer, 0) == 0);
    image.resize(uint2(11, 5));
    assert(readBack(image.buffer, 54) == 410);
    var empty = Tensor<uint, 3>(uint3(0xffffffffu, 0xffffffffu, 0));
    assert(empty.elementCount() == 0 && empty.capacity == 1);
    empty.resize(uint3(0, 2, 3));
    empty.resize(uint3(2, 0, 3));
    assert(empty.elementCount() == 0 && empty.capacity == 1);
    empty.resize(uint3(2, 3, 4));
    assert(empty.elementCount() == 24 && empty.capacity == 24);
    Grid(empty.shape)
        .dispatch(
            (uint3 p) =>
            { empty[p] = 42; });
    assert(readBack(empty.buffer, 23) == 42);
}
