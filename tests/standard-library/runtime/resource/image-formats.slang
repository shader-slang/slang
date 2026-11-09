// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

// TEST(render):INTERPRET(hgvm): -api d3d12
// TEST(render):INTERPRET(hgvm): -api vulkan

import slang.runtime;

[shader("compute")]
[numthreads(1, 1, 1)]
void sampleImage(RWTexture2D<float4> image, RWStructuredBuffer<float4> result)
{
    result[0] = image[uint2(9, 4)];
}

void checkPixels(float4 actual, float4 expected, float tolerance)
{
    if (any(actual < expected - float4(tolerance)) || any(actual > expected + float4(tolerance)))
        fail("Loaded image pixels do not match the selected format");
}

/// Exercises runtime format selection and all three loading overloads on extensionless paths.
void checkFormat(String path, ImageFileFormat fileFormat, float4 expected, float tolerance)
{
    let source = allocateTexture2D<float4>(19, 9);
    source.clear(float4(-0.25, 0.25, 1.5, 0.5));
    storeTexture2D(path, source, fileFormat);
    let inferred = loadTexture2D(path, fileFormat);
    let sized = loadTexture2D(path, uint2(19, 9), fileFormat);
    let allocated = allocateTexture2D<float4>(19, 9);
    loadTexture2D(path, allocated, fileFormat);
    if (any(inferred.getDimensions() != uint2(19, 9)))
        fail("Image dimensions were not inferred correctly");
    let result = RWStructuredBuffer<float4>::alloc(1);
    dispatch(sampleImage, uint3(1, 1, 1))(inferred, result);
    checkPixels(readBack(result, 0), expected, tolerance);
    dispatch(sampleImage, uint3(1, 1, 1))(sized, result);
    checkPixels(readBack(result, 0), expected, tolerance);
    dispatch(sampleImage, uint3(1, 1, 1))(allocated, result);
    checkPixels(readBack(result, 0), expected, tolerance);
}

[host]
void main()
{
    checkFormat("image-formats-exr.actual", ImageFileFormat.EXR, float4(-0.25, 0.25, 1.5, 0.5), 0);
    checkFormat(
        "image-formats-png.actual",
        ImageFileFormat.PNG,
        float4(0, 64.0 / 255, 1, 128.0 / 255),
        0.000001);
    checkFormat("image-formats-jpg.actual", ImageFileFormat.JPG, float4(0, 0.25, 1, 1), 0.03);
    checkFormat(
        "image-formats-bmp.actual",
        ImageFileFormat.BMP,
        float4(0, 64.0 / 255, 1, 128.0 / 255),
        0.000001);

    // A declared physical texture format must still work with the high-level format parameter.
    let explicitTexture = allocateTexture2D<float4, TextureFormat.rgba32Float>(19, 9);
    explicitTexture.clear(float4(0.25));
    storeTexture2D("image-formats-explicit.actual", explicitTexture, ImageFileFormat.PNG);
    loadTexture2D("image-formats-explicit.actual", explicitTexture, ImageFileFormat.PNG);
}
