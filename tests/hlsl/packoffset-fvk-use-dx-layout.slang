//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-slang -compute -shaderobj -output-using-type -fvk-use-dx-layout
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type -fvk-use-dx-layout
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -profile cs_5_0 -entry computeMain -line-directive-mode none -fvk-use-dx-layout
//TEST:SIMPLE(filecheck=GLSL): -target glsl -profile glsl_450 -stage compute -entry computeMain -line-directive-mode none -fvk-use-dx-layout
//TEST:SIMPLE(filecheck=SPIRV): -target spirv -profile cs_5_0 -entry computeMain -line-directive-mode none -fvk-use-dx-layout

// Metal does not support custom data layout.
//DISABLE_TEST(compute):COMPARE_COMPUTE:-slang -shaderobj -mtl


//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=16):out,name=outputBuffer
RWStructuredBuffer<float4> outputBuffer;

//TEST_INPUT:set Constants.v0={1.0,2.0,3.0,4.0}
//TEST_INPUT:set Constants.v1=8.0
//TEST_INPUT:set Constants.v2={5.0,6.0,7.0}

cbuffer Constants
{
    float4 v0;
    float  v1;
    float3 v2;
};
// HLSL: cbuffer
// HLSL: {
// HLSL: {{.*}} : packoffset(c0);
// HLSL: {{.*}} : packoffset(c1);
// HLSL: {{.*}} : packoffset(c1.y);
// HLSL: }
// GLSL: layout(offset = 0)
// GLSL: layout(offset = 16)
// GLSL: layout(offset = 20)

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = float4(v2, v1);
}
