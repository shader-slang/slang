// Test that matrix types do NOT use OpBitcast per SPIR-V spec
// Per spec: OpBitcast only allows "scalar or vector of numerical type"

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly

// CHECK-NOT: OpBitcast %mat4v4float
// CHECK-NOT: OpBitcast %mat3v3float
// CHECK-NOT: OpBitcast %mat2v2float

// CHECK-DAG: OpBitcast %float
// CHECK-DAG: OpCompositeExtract

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=OUT):-vk -compute -emit-spirv-directly -output-using-type

//TEST_INPUT:ubuffer(data=[1065353216 1073741824 1077936128 1082130432 1084227584 1086324736 1088421888 1090519040 1091567616 1092616192 1093664768 1094713344 1095761920 1096810496 1097859072 1098907648], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// OUT: 1065353216
// OUT: 1065353216
// OUT: 1065353216

[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // Use input data to prevent constant folding

    // Test: float4x4 matrix bitcast
    // Construct a matrix-like structure using uint values
    uint4x4 umat4x4;
    umat4x4[0] = uint4(inputBuffer[0], inputBuffer[1], inputBuffer[2], inputBuffer[3]);
    umat4x4[1] = uint4(inputBuffer[4], inputBuffer[5], inputBuffer[6], inputBuffer[7]);
    umat4x4[2] = uint4(inputBuffer[8], inputBuffer[9], inputBuffer[10], inputBuffer[11]);
    umat4x4[3] = uint4(inputBuffer[12], inputBuffer[13], inputBuffer[14], inputBuffer[15]);

    float4x4 fmat4x4 = bit_cast<float4x4>(umat4x4);

    // Test: float3x3 matrix bitcast
    uint3x3 umat3x3;
    umat3x3[0] = uint3(inputBuffer[0], inputBuffer[1], inputBuffer[2]);
    umat3x3[1] = uint3(inputBuffer[3], inputBuffer[4], inputBuffer[5]);
    umat3x3[2] = uint3(inputBuffer[6], inputBuffer[7], inputBuffer[8]);

    float3x3 fmat3x3 = bit_cast<float3x3>(umat3x3);

    // Test: float2x2 matrix bitcast
    uint2x2 umat2x2;
    umat2x2[0] = uint2(inputBuffer[0], inputBuffer[1]);
    umat2x2[1] = uint2(inputBuffer[2], inputBuffer[3]);

    float2x2 fmat2x2 = bit_cast<float2x2>(umat2x2);

    // Store results to prevent optimization
    outputBuffer[0] = asuint(fmat4x4[0][0]);
    outputBuffer[1] = asuint(fmat3x3[0][0]);
    outputBuffer[2] = asuint(fmat2x2[0][0]);
}
