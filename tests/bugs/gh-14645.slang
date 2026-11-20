// Storing to one component of an `RWTexture2D` texel must not abort compilation on SPIR-V,
// GLSL or Metal, and must behave as a read-modify-write that only updates the addressed
// component. This covers both store forms of the legalized path: a literal component index
// (the swizzle fast path) and a runtime index from a cbuffer (the per-component select
// fallback). All textures are initialized to all-ones; the literal store sets component 1
// to 0.5 and the runtime store sets component `Params.channel` (2) to 0.25, so the other
// components staying at 1.0 proves they were preserved. The out-of-range runtime store on
// the second texture must leave its texel unchanged, and a runtime index that selects the
// padding lane past a `float3` element type must leave that texel unchanged too.

//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=MTL):-target metal -entry computeMain -stage compute

//TEST_INPUT:cbuffer(data=[2 0 0 0]):name=Params
cbuffer Params
{
    uint channel;
}

//TEST_INPUT:RWTexture2D(format=RGBA32Float, size=4, content = one, mipMaps = 1):name=outputTexture
RWTexture2D<float4> outputTexture;

//TEST_INPUT:RWTexture2D(format=RGBA32Float, size=4, content = one, mipMaps = 1):name=outputTexture2
RWTexture2D<float4> outputTexture2;

//TEST_INPUT:RWTexture2D(format=RGBA32Float, size=4, content = one, mipMaps = 1):name=outputTexture3
RWTexture2D<float3> outputTexture3;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(4, 4, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputTexture[tid.xy][1] = 0.5;
    outputTexture[tid.xy][channel] = 0.25;
    outputTexture2[tid.xy][channel + 8] = 0.25;
    // Channel + 1 selects the padding lane past the float3 element type; the store must
    // leave the texel's components unchanged.
    outputTexture3[tid.xy][channel + 1] = 0.25;

    outputBuffer[0] = uint(true
        && all(outputTexture[tid.xy] == float4(1.0, 0.5, 0.25, 1.0))
        && all(outputTexture2[tid.xy] == float4(1.0, 1.0, 1.0, 1.0))
        && all(outputTexture3[tid.xy] == float3(1.0, 1.0, 1.0))
        );
    //BUF: 1

    //SPIRV: OpImageRead
    //SPIRV: OpImageWrite
    //GLSL: imageLoad
    //GLSL: imageStore
    //MTL: .write(
}
