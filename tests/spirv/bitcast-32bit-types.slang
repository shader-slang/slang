// Test that 32-bit types use OpBitcast directly in SPIR-V
// Tests float <-> int <-> uint in scalar, vec2, vec3, vec4 forms

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly

// Per SPIR-V spec: these types MUST use direct OpBitcast (no lowering/splitting)
// Scalar: float, int, uint
// CHECK-DAG: OpBitcast %float %
// CHECK-DAG: OpBitcast %int %
// CHECK-DAG: OpBitcast %uint %

// Vec2: float2, int2, uint2
// CHECK-DAG: OpBitcast %v2float %
// CHECK-DAG: OpBitcast %v2int %

// Vec3: float3, int3, uint3
// CHECK-DAG: OpBitcast %v3float %
// CHECK-DAG: OpBitcast %v3int %

// Vec4: float4, int4, uint4
// CHECK-DAG: OpBitcast %v4float %
// CHECK-DAG: OpBitcast %v4int %

// Also check that we have uint bitcasts back from float/int
// CHECK-DAG: OpBitcast %uint %

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=OUT):-vk -compute -emit-spirv-directly -output-using-type

//TEST_INPUT:ubuffer(data=[1078523331 1073741824 1077936128 1082130432 286331153 572662306 1065353216 1073741824 1077936128 1084227584], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=floatBuffer
RWStructuredBuffer<float> floatBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=intBuffer
RWStructuredBuffer<int> intBuffer;

// OUT: 1078523331
// OUT: 1078523331
// OUT: 1078523331
// OUT: 3221225472
// OUT: 1073741824
// OUT: 3246391296
// OUT: 1082130432
// OUT: 3196059648
// OUT: 1065353216
// OUT: 1077936128
// OUT: 1077936128
// OUT: 286331153

[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // Scalar: uint -> float -> uint, uint -> int -> uint
    uint u32 = inputBuffer[0];
    float f32 = bit_cast<float>(u32);
    // Store to float buffer to force the bitcast to be preserved
    floatBuffer[0] = f32;
    // Bitcast back from float to uint
    uint u32_from_f = bit_cast<uint>(floatBuffer[0]);
    
    int i32 = bit_cast<int>(u32);
    // Store to int buffer to force the bitcast to be preserved
    intBuffer[0] = i32;
    // Bitcast back from int to uint
    uint u32_from_i = bit_cast<uint>(intBuffer[0]);

    // Vec2: uint2 -> float2 -> uint2, uint2 -> int2 -> uint2
    uint2 u32v2 = uint2(inputBuffer[1], inputBuffer[2]);
    float2 f32v2 = bit_cast<float2>(u32v2);
    uint2 u32v2_from_f = bit_cast<uint2>(f32v2);
    int2 i32v2 = bit_cast<int2>(u32v2);
    uint2 u32v2_from_i = bit_cast<uint2>(i32v2);

    // Vec3: uint3 -> float3 -> uint3, uint3 -> int3 -> uint3
    uint3 u32v3 = uint3(inputBuffer[3], inputBuffer[4], inputBuffer[5]);
    float3 f32v3 = bit_cast<float3>(u32v3);
    uint3 u32v3_from_f = bit_cast<uint3>(f32v3);
    int3 i32v3 = bit_cast<int3>(u32v3);
    uint3 u32v3_from_i = bit_cast<uint3>(i32v3);

    // Vec4: uint4 -> float4 -> uint4, uint4 -> int4 -> uint4
    uint4 u32v4 = uint4(inputBuffer[6], inputBuffer[7], inputBuffer[8], inputBuffer[9]);
    float4 f32v4 = bit_cast<float4>(u32v4);
    uint4 u32v4_from_f = bit_cast<uint4>(f32v4);
    int4 i32v4 = bit_cast<int4>(u32v4);
    uint4 u32v4_from_i = bit_cast<uint4>(i32v4);

    // Store results - use all bitcast results to prevent optimization
    outputBuffer[0] = u32_from_f;
    outputBuffer[1] = u32_from_i;
    outputBuffer[2] = asuint(f32);  // Use the original scalar float bitcast result
    outputBuffer[3] = u32v2_from_f.x + asuint(f32v2.x) + uint(i32v2.x);  // Use f32v2, i32v2, and their uint casts
    outputBuffer[4] = u32v2_from_i.x;
    outputBuffer[5] = u32v3_from_f.x + asuint(f32v3.x) + uint(i32v3.x);  // Use f32v3, i32v3, and their uint casts
    outputBuffer[6] = u32v3_from_i.x;
    outputBuffer[7] = u32v4_from_f.x + asuint(f32v4.x) + uint(i32v4.x);  // Use f32v4, i32v4, and their uint casts
    outputBuffer[8] = u32v4_from_i.x;
    outputBuffer[9] = u32v2_from_i.y;
    outputBuffer[10] = u32v2_from_f.y;
    outputBuffer[11] = u32v3_from_f.y;
}
