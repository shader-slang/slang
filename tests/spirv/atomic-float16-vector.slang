//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -emit-spirv-directly
// RHI does not yet expose a feature gate for VK_NV_shader_atomic_float16_vector, so this runtime
// check stays disabled while the active SIMPLE test verifies SPIR-V capability and instruction use.
//DISABLE_TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=RUNTIME):-vk -compute -entry computeMain -emit-spirv-directly -capability spvAtomicFloat16VectorNV -output-using-type

//TEST_INPUT:ubuffer(stride=4, count=12):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(stride=4, count=4):name=workBuffer
RWStructuredBuffer<half2> workBuffer;
//TEST_INPUT:ubuffer(stride=8, count=4):name=workBuffer4
RWStructuredBuffer<half4> workBuffer4;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    workBuffer[0] = half2(2.0h, 4.0h);
    workBuffer[1] = half2(5.0h, 1.0h);
    workBuffer[2] = half2(5.0h, 1.0h);
    workBuffer[3] = half2(8.0h, 9.0h);

    workBuffer4[0] = half4(1.0h, 2.0h, 3.0h, 4.0h);
    workBuffer4[1] = half4(5.0h, 1.0h, 7.0h, 2.0h);
    workBuffer4[2] = half4(5.0h, 1.0h, 7.0h, 2.0h);
    workBuffer4[3] = half4(8.0h, 9.0h, 10.0h, 11.0h);

    half2 originalAdd;
    half2 originalMin;
    half2 originalMax;
    half2 originalExchange;
    half4 originalAdd4;
    half4 originalMin4;
    half4 originalMax4;
    half4 originalExchange4;

    // Direct atomic operation on half2 should trigger the SPV_NV_shader_atomic_fp16_vector extension
    originalAdd = __atomic_add(workBuffer[0], half2(1.0h, 2.0h));
    originalMin = __atomic_min(workBuffer[1], half2(1.0h, 2.0h));
    originalMax = __atomic_max(workBuffer[2], half2(1.0h, 2.0h));
    originalExchange = __atomic_exchange(workBuffer[3], half2(3.0h, 4.0h));
    originalAdd4 = __atomic_add(workBuffer4[0], half4(1.0h, 2.0h, 3.0h, 4.0h));
    originalMin4 = __atomic_min(workBuffer4[1], half4(1.0h, 2.0h, 3.0h, 4.0h));
    originalMax4 = __atomic_max(workBuffer4[2], half4(1.0h, 2.0h, 3.0h, 4.0h));
    originalExchange4 = __atomic_exchange(workBuffer4[3], half4(5.0h, 6.0h, 7.0h, 8.0h));

    half4 original4Sum = originalAdd4 + originalMin4 + originalMax4 + originalExchange4;
    half4 final4Sum = workBuffer4[0] + workBuffer4[1] + workBuffer4[2] + workBuffer4[3];

    outputBuffer[0] = float(originalAdd.x + originalMin.x + originalMax.x + originalExchange.x);
    outputBuffer[1] = float(originalAdd.y + originalMin.y + originalMax.y + originalExchange.y);
    outputBuffer[2] = float(original4Sum.x);
    outputBuffer[3] = float(original4Sum.y);
    outputBuffer[4] = float(original4Sum.z);
    outputBuffer[5] = float(original4Sum.w);
    outputBuffer[6] = float(workBuffer[0].x + workBuffer[1].x + workBuffer[2].x + workBuffer[3].x);
    outputBuffer[7] = float(workBuffer[0].y + workBuffer[1].y + workBuffer[2].y + workBuffer[3].y);
    outputBuffer[8] = float(final4Sum.x);
    outputBuffer[9] = float(final4Sum.y);
    outputBuffer[10] = float(final4Sum.z);
    outputBuffer[11] = float(final4Sum.w);

    // RUNTIME: type: float
    // RUNTIME-NEXT: 20.000000
    // RUNTIME-NEXT: 15.000000
    // RUNTIME-NEXT: 19.000000
    // RUNTIME-NEXT: 13.000000
    // RUNTIME-NEXT: 27.000000
    // RUNTIME-NEXT: 19.000000
    // RUNTIME-NEXT: 12.000000
    // RUNTIME-NEXT: 13.000000
    // RUNTIME-NEXT: 13.000000
    // RUNTIME-NEXT: 13.000000
    // RUNTIME-NEXT: 23.000000
    // RUNTIME-NEXT: 22.000000
}

// CHECK-NOT: OpCapability AtomicFloat16AddEXT
// CHECK: OpCapability AtomicFloat16VectorNV
// CHECK-NOT: OpCapability AtomicFloat16AddEXT
// CHECK: OpExtension "SPV_NV_shader_atomic_fp16_vector"
// CHECK-NOT: OpExtension "SPV_EXT_shader_atomic_float16_add"
// CHECK-COUNT-1: OpAtomicFAddEXT %v2half
// CHECK-COUNT-1: OpAtomicFMinEXT %v2half
// CHECK-COUNT-1: OpAtomicFMaxEXT %v2half
// CHECK-COUNT-1: OpAtomicExchange %v2half
// CHECK-COUNT-1: OpAtomicFAddEXT %v4half
// CHECK-COUNT-1: OpAtomicFMinEXT %v4half
// CHECK-COUNT-1: OpAtomicFMaxEXT %v4half
// CHECK-COUNT-1: OpAtomicExchange %v4half
