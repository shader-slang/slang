//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type
//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type -xslang -DMIX_PACKOFFSET
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -profile cs_5_0 -entry computeMain -line-directive-mode none
//TEST:SIMPLE(filecheck=GLSL): -target glsl -profile glsl_450 -stage compute -entry computeMain -line-directive-mode none
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -profile cs_5_0 -entry computeMain -line-directive-mode none -DMIX_PACKOFFSET
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type -dx12 -use-dxil -xslang -DMIX_PACKOFFSET

// Metal does not support custom data layout.
//DISABLE_TEST(compute):COMPARE_COMPUTE:-slang -shaderobj -mtl


//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:set Constants.v0={1.0,2.0,3.0,4.0}
//TEST_INPUT:set Constants.v1={5.0,6.0,7.0}
//TEST_INPUT:set Constants.v2=8.0
//TEST_INPUT:set Constants.v3={9.0,10.0,11.0,12.0}

cbuffer Constants
{
    float4 v0 : packoffset(c0);
    float3 v1 : packoffset(c1);
    float v2 : packoffset(c1.w);
    float4 v3 : packoffset(c3);
#ifdef MIX_PACKOFFSET
    // Mixing packed and unpacked offsets is
    // not allowed by fxc, but is allowed by dxc.
    float4 v4;
#endif
};
// HLSL: cbuffer
// HLSL: {
// HLSL: {{.*}} : packoffset(c0);
// HLSL: {{.*}} : packoffset(c1);
// HLSL: {{.*}} : packoffset(c1.w);
// HLSL: {{.*}} : packoffset(c3);
// HLSL: }
// GLSL: layout(offset = 0)
// GLSL: layout(offset = 16)
// GLSL: layout(offset = 28)
// GLSL: layout(offset = 48)
// SPIRV: OpMemberDecorate {{.*}} 0 Offset 0
// SPIRV: OpMemberDecorate {{.*}} 1 Offset 16
// SPIRV: OpMemberDecorate {{.*}} 2 Offset 28
// SPIRV: OpMemberDecorate {{.*}} 3 Offset 48
// SPIRV: OpMemberDecorate {{.*}} 4 Offset 64

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = v2;
}
