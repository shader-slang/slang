// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST:INTERPRET(hgvm): -api vulkan
// TEST:INTERPRET(hgvm): -api cuda

import "../support/image_compare.slang";
import slang.compute.buffer_utils;
import slang.compute.texture_utils;
import slang.runtime.core.simple_assert;
import slang.runtime.core.simple_string;
import slang.runtime.resource.simple_texture;

struct AddFloat : IFunc<float, float, float>
{
    float operator()(float left, float right) { return left + right; }
}

[shader("compute")]
[numthreads(8, 8, 1)]
void initializeReductionTexture(RWTexture2D<float> texture, uint3 threadID: SV_DispatchThreadID)
{
    uint width;
    uint height;
    texture.GetDimensions(width, height);
    if (threadID.x >= width || threadID.y >= height)
        return;
    texture[threadID.xy] = float(threadID.y * width + threadID.x + 1);
}

[shader("compute")]
[numthreads(64, 1, 1)]
void initializeReductionBuffer(
    RWStructuredBuffer<float> buffer,
    uniform uint count,
    uint3 threadID: SV_DispatchThreadID)
{
    if (threadID.x < count)
        buffer[threadID.x] = 1.0;
}

[shader("compute")]
[numthreads(8, 8, 1)]
void initializeComparisonTextures(
    RWTexture2D<float4> actual,
    RWTexture2D<float4> expected,
    uint3 threadID: SV_DispatchThreadID)
{
    uint width;
    uint height;
    actual.GetDimensions(width, height);
    if (threadID.x >= width || threadID.y >= height)
        return;
    expected[threadID.xy] = float4(0.0);
    actual[threadID.xy] = all(threadID.xy == uint2(0)) ? float4(1.0, 0.0, 0.0, 0.0) : float4(0.0);
}

void assertNear(float actual, float expected, float tolerance)
{
    if (actual < expected - tolerance || actual > expected + tolerance)
        fail(format("Expected {} but got {}", expected, actual));
}

[host]
void main()
{
    uint2 dimensions = uint2(17, 9);
    RWTexture2D<float> texture = allocateTexture2D<float>(dimensions.x, dimensions.y);
    dispatch(initializeReductionTexture, uint3(3, 2, 1))(texture);
    RWStructuredBuffer<float> total = RWStructuredBuffer<float>::alloc(1);
    texture.reduceTo(total, AddFloat());
    assertNear(readBack(total, 0), 11781.0, 0.01);

    texture.mapInPlace((float value) => value * 2.0);
    texture.reduceTo(total, AddFloat());
    assertNear(readBack(total, 0), 23562.0, 0.01);

    texture.mapInPlaceIndexed(
        (uint2 location, float value) => all(location == uint2(0)) ? value + 1.0 : value);
    texture.reduceTo(total, AddFloat());
    assertNear(readBack(total, 0), 23563.0, 0.01);

    RWTexture2D<float> mappedTexture = allocateTexture2D<float>(dimensions.x, dimensions.y);
    texture.mapTo(mappedTexture, (float value) => value * 0.5);
    mappedTexture.reduceTo(total, AddFloat());
    assertNear(readBack(total, 0), 11781.5, 0.01);

    RWTexture2D<float> indexedMappedTexture = allocateTexture2D<float>(dimensions.x, dimensions.y);
    mappedTexture.mapToIndexed(
        indexedMappedTexture,
        (uint2 location, float value) => all(location == uint2(0)) ? value - 0.5 : value);
    indexedMappedTexture.reduceTo(total, AddFloat());
    assertNear(readBack(total, 0), 11781.0, 0.01);

    uint bufferCount = 20000;
    RWStructuredBuffer<float> buffer = RWStructuredBuffer<float>::alloc(bufferCount);
    dispatch(initializeReductionBuffer, uint3((bufferCount + 63) / 64, 1, 1))(buffer, bufferCount);
    buffer.reduceTo(total, AddFloat());
    assertNear(readBack(total, 0), float(bufferCount), 0.01);

    buffer.mapInPlace((float value) => value * 2.0);
    buffer.reduceTo(total, AddFloat());
    assertNear(readBack(total, 0), float(bufferCount * 2), 0.01);

    buffer.mapInPlaceIndexed((uint index, float value) => index == 0 ? value + 1.0 : value);
    buffer.reduceTo(total, AddFloat());
    assertNear(readBack(total, 0), float(bufferCount * 2 + 1), 0.01);

    RWStructuredBuffer<float> mappedBuffer = RWStructuredBuffer<float>::alloc(bufferCount);
    buffer.mapTo(mappedBuffer, (float value) => value * 0.5);
    mappedBuffer.reduceTo(total, AddFloat());
    assertNear(readBack(total, 0), float(bufferCount) + 0.5, 0.01);

    RWStructuredBuffer<float> indexedMappedBuffer = RWStructuredBuffer<float>::alloc(bufferCount);
    mappedBuffer.mapToIndexed(
        indexedMappedBuffer,
        (uint index, float value) => index == 0 ? value - 0.5 : value);
    indexedMappedBuffer.reduceTo(total, AddFloat());
    assertNear(readBack(total, 0), float(bufferCount), 0.01);

    RWStructuredBuffer<float> visitedBuffer = RWStructuredBuffer<float>::alloc(bufferCount);
    mappedBuffer.mapIndexed(
        bufferCount,
        (uint index, float value) =>
        { visitedBuffer[index] = index == 0 ? value - 0.5 : value; });
    visitedBuffer.reduceTo(total, AddFloat());
    assertNear(readBack(total, 0), float(bufferCount), 0.01);

    buffer.reduceTo(total, 0, AddFloat());
    assertNear(readBack(total, 0), float(bufferCount), 0.01);

    RWStructuredBuffer<float> single = RWStructuredBuffer<float>::alloc(1);
    dispatch(initializeReductionBuffer, uint3(1, 1, 1))(single, 1);
    single.reduceTo(total, AddFloat());
    assertNear(readBack(total, 0), 1.0, 0.000001);

    RWTexture2D<float4> actual = allocateTexture2D<float4>(dimensions.x, dimensions.y);
    RWTexture2D<float4> expected = allocateTexture2D<float4>(dimensions.x, dimensions.y);
    dispatch(initializeComparisonTextures, uint3(3, 2, 1))(actual, expected);
    float mse = computeImageMSE(actual, expected);
    assertNear(mse, 1.0 / 612.0, 0.000001);
}
