//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -xslang -matrix-layout-row-major
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -xslang -matrix-layout-column-major

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name outputBuffer
RWStructuredBuffer<int> outputBuffer;

// Difference from hlsl-intrinsic/matrix-select.slang
// is that GLSL/SPIRV does not support 1 column/row matrices
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    float s = 0;

    float2x2 a2x2 = (float2x2) 0;
    float2x2 b2x2 = (float2x2) 1;
    float2x2 c2x2 = select(bool2x2(true), a2x2, b2x2);
    s += c2x2[0][0];

    float2x3 a2x3 = (float2x3) 0;
    float2x3 b2x3 = (float2x3) 1;
    float2x3 c2x3 = select(bool2x3(true), a2x3, b2x3);
    s += c2x3[0][0];

    float2x4 a2x4 = (float2x4) 0;
    float2x4 b2x4 = (float2x4) 1;
    float2x4 c2x4 = select(bool2x4(true), a2x4, b2x4);
    s += c2x4[0][0];

    float3x2 a3x2 = (float3x2) 0;
    float3x2 b3x2 = (float3x2) 1;
    float3x2 c3x2 = select(bool3x2(true), a3x2, b3x2);
    s += c3x2[0][0];

    float3x3 a3x3 = (float3x3) 0;
    float3x3 b3x3 = (float3x3) 1;
    float3x3 c3x3 = select(bool3x3(true), a3x3, b3x3);
    s += c3x3[0][0];

    float3x4 a3x4 = (float3x4) 0;
    float3x4 b3x4 = (float3x4) 1;
    float3x4 c3x4 = select(bool3x4(true), a3x4, b3x4);
    s += c3x4[0][0];

    float4x2 a4x2 = (float4x2) 0;
    float4x2 b4x2 = (float4x2) 1;
    float4x2 c4x2 = select(bool4x2(true), a4x2, b4x2);
    s += c4x2[0][0];

    float4x3 a4x3 = (float4x3) 0;
    float4x3 b4x3 = (float4x3) 1;
    float4x3 c4x3 = select(bool4x3(true), a4x3, b4x3);
    s += c4x3[0][0];

    float4x4 a4x4 = (float4x4) 0;
    float4x4 b4x4 = (float4x4) 1;
    float4x4 c4x4 = select(bool4x4(true), a4x4, b4x4);
    s += c4x4[0][0];

    outputBuffer[0] = (int) s;
    // BUF: 1
}
