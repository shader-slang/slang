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
// CHECK-DAG: OpBitcast %v2uint %

// Vec3: float3, int3, uint3
// CHECK-DAG: OpBitcast %v3float %
// CHECK-DAG: OpBitcast %v3int %
// CHECK-DAG: OpBitcast %v3uint %

// Vec4: float4, int4, uint4
// CHECK-DAG: OpBitcast %v4float %
// CHECK-DAG: OpBitcast %v4int %
// CHECK-DAG: OpBitcast %v4uint %

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=OUT):-vk -compute -emit-spirv-directly -output-using-type

//TEST_INPUT:ubuffer(data=[1078523331 1073741824 1077936128 1082130432 286331153 572662306 1065353216 1073741824 1077936128 1084227584], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// OUT: 1078523331
// OUT: 1078523331
// OUT: 1073741824
// OUT: 1073741824
// OUT: 1082130432
// OUT: 1082130432
// OUT: 1065353216
// OUT: 1065353216

[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // Scalar: uint -> float -> uint, uint -> int -> uint
    uint u32 = inputBuffer[0];
    float f32 = bit_cast<float>(u32);
    uint u32_from_f = bit_cast<uint>(f32);
    int i32 = bit_cast<int>(u32);

    // Vec2: uint2 -> float2 -> uint2, uint2 -> int2
    uint2 u32v2 = uint2(inputBuffer[1], inputBuffer[2]);
    float2 f32v2 = bit_cast<float2>(u32v2);
    uint2 u32v2_from_f = bit_cast<uint2>(f32v2);
    int2 i32v2 = bit_cast<int2>(u32v2);

    // Vec3: uint3 -> float3, uint3 -> int3 -> uint3
    uint3 u32v3 = uint3(inputBuffer[3], inputBuffer[4], inputBuffer[5]);
    float3 f32v3 = bit_cast<float3>(u32v3);
    int3 i32v3 = bit_cast<int3>(u32v3);
    uint3 u32v3_from_i = bit_cast<uint3>(i32v3);

    // Vec4: uint4 -> float4, uint4 -> int4 -> uint4
    uint4 u32v4 = uint4(inputBuffer[6], inputBuffer[7], inputBuffer[8], inputBuffer[9]);
    float4 f32v4 = bit_cast<float4>(u32v4);
    int4 i32v4 = bit_cast<int4>(u32v4);
    uint4 u32v4_from_i = bit_cast<uint4>(i32v4);

    // Store results
    outputBuffer[0] = u32_from_f;
    outputBuffer[1] = uint(i32);
    outputBuffer[2] = u32v2_from_f.x;
    outputBuffer[3] = uint(i32v2.x);
    outputBuffer[4] = asuint(f32v3.x);
    outputBuffer[5] = u32v3_from_i.x;
    outputBuffer[6] = asuint(f32v4.x);
    outputBuffer[7] = u32v4_from_i.x;
}
