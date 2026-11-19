// Storing to one component of an `RWTexture2D` texel at a runtime index
// (from a cbuffer) must not abort compilation on SPIR-V, GLSL or Metal, and
// must behave as a read-modify-write that only updates the addressed
// component. The texture is initialized to all-ones and component
// `Params.channel` (2) is set to 0.25, so the other three components
// staying at 1.0 proves they were preserved by the store.

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

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(4, 4, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputTexture[tid.xy][channel] = 0.25;

    outputBuffer[0] = uint(all(outputTexture[tid.xy] == float4(1.0, 1.0, 0.25, 1.0)));
    //BUF: 1

    //SPIRV: OpImageWrite
    //GLSL: imageStore
    //MTL: write
}
