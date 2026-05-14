//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain -stage compute -line-directive-mode none
//TEST:SIMPLE(filecheck=MSL): -target metal -entry computeMain -stage compute -line-directive-mode none

// Explicit texel format descriptor types should provide both the storage
// image format and the data type used by texture operations.

RWTexture2D<rgba16f> floatImage;
RWTexture2D<rgba8ui> uintImage;

// GLSL: layout(rgba16f)
// GLSL: uniform image2D floatImage
// GLSL: layout(rgba8ui)
// GLSL: uniform uimage2D uintImage
// GLSL: vec4
// GLSL: uvec4

// SPIRV: OpTypeImage {{.*}} Rgba16f
// SPIRV: OpTypeImage {{.*}} Rgba8ui

// MSL: texture2d<float, access::read_write>
// MSL: texture2d<uint, access::read_write>

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint2 pixel = dispatchThreadID.xy;

    float4 floatValue = floatImage[pixel];
    uint4 uintValue = uintImage[pixel];

    floatImage[pixel] = floatValue;
    uintImage[pixel] = uintValue;
}
