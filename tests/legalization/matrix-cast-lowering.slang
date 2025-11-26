//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -vk -shaderobj -xslang -emit-spirv-directly -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -vk -shaderobj -xslang -emit-spirv-via-glsl -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -mtl -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -wgpu -shaderobj -output-using-type

// Test matrix type legalization for casts between lowered and non-lowered matrices.
//
// On SPIR-V, Metal, and WGSL targets, matrices with uint/int/bool elements are lowered to
// arrays of vectors, while float matrices remain as native matrix types. This test verifies
// that casts from lowered matrices to non-lowered matrices are properly handled.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    // CHECK: type: float

    // uint4x4 (lowered) to float4x4 (not lowered) cast
    float4x4 mat1 = uint4x4(1);
    float4 result1 = mul(mat1, float4(1, 0, 0, 0));

    // CHECK-NEXT: 1.000000
    // CHECK-NEXT: 1.000000
    // CHECK-NEXT: 1.000000
    // CHECK-NEXT: 1.000000
    outputBuffer[0] = result1.x;
    outputBuffer[1] = result1.y;
    outputBuffer[2] = result1.z;
    outputBuffer[3] = result1.w;

    // int4x4 (lowered) to float4x4 (not lowered) cast
    float4x4 mat2 = int4x4(2);
    float4 result2 = mul(mat2, float4(1, 0, 0, 0));

    // CHECK-NEXT: 2.000000
    // CHECK-NEXT: 2.000000
    // CHECK-NEXT: 2.000000
    // CHECK-NEXT: 2.000000
    outputBuffer[4] = result2.x;
    outputBuffer[5] = result2.y;
    outputBuffer[6] = result2.z;
    outputBuffer[7] = result2.w;

    // Test different matrix sizes
    float2x2 mat3 = uint2x2(1);
    float2 result3 = mul(mat3, float2(1, 0));

    // CHECK-NEXT: 1.000000
    // CHECK-NEXT: 1.000000
    outputBuffer[8] = result3.x;
    outputBuffer[9] = result3.y;

    float3x3 mat4 = uint3x3(1);
    float3 result4 = mul(mat4, float3(1, 0, 0));

    // CHECK-NEXT: 1.000000
    // CHECK-NEXT: 1.000000
    // CHECK-NEXT: 1.000000
    outputBuffer[10] = result4.x;
    outputBuffer[11] = result4.y;
    outputBuffer[12] = result4.z;

    // Test explicit cast syntax
    float4x4 mat5 = (float4x4)uint4x4(1);
    float4 result5 = mul(mat5, float4(1, 0, 0, 0));

    // CHECK-NEXT: 1.000000
    // CHECK-NEXT: 1.000000
    // CHECK-NEXT: 1.000000
    // CHECK-NEXT: 1.000000

    outputBuffer[13] = result5.x;
    outputBuffer[14] = result5.y;
    outputBuffer[15] = result5.z;
    outputBuffer[16] = result5.w;

    // Test with variables
    uint4x4 uintMat = uint4x4(1);
    float4x4 floatMat = float4x4(uintMat);
    float4 result6 = mul(floatMat, float4(1, 0, 0, 0));

    // CHECK-NEXT: 1.000000
    // CHECK-NEXT: 1.000000
    // CHECK-NEXT: 1.000000
    // CHECK-NEXT: 1.000000
    outputBuffer[17] = result6.x;
    outputBuffer[18] = result6.y;
    outputBuffer[19] = result6.z;
    outputBuffer[20] = result6.w;

    // Test casts between two lowered matrix types and a non-lowered matrix type
    int4x4 intMat = uint4x4(1);
    float4x4 floatMat2 = float4x4(intMat);
    float4 result7 = mul(floatMat2, float4(1, 0, 0, 0));

    // CHECK-NEXT: 1.000000
    // CHECK-NEXT: 1.000000
    // CHECK-NEXT: 1.000000
    // CHECK-NEXT: 1.000000
    outputBuffer[21] = result7.x;
    outputBuffer[22] = result7.y;
    outputBuffer[23] = result7.z;
    outputBuffer[24] = result7.w;

    uint4x4 uintMat2 = int4x4(2);
    float4x4 floatMat3 = float4x4(uintMat2);
    float4 result8 = mul(floatMat3, float4(1, 0, 0, 0));

    // CHECK-NEXT: 2.000000
    // CHECK-NEXT: 2.000000
    // CHECK-NEXT: 2.000000
    // CHECK-NEXT: 2.000000
    outputBuffer[25] = result8.x;
    outputBuffer[26] = result8.y;
    outputBuffer[27] = result8.z;
    outputBuffer[28] = result8.w;
}

