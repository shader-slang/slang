//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):name=workBuffer
RWStructuredBuffer<half2> workBuffer;
RWStructuredBuffer<half4> workBuffer4;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
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

    outputBuffer[0] = float(originalAdd.x + originalMin.x + originalMax.x + originalExchange.x);
    outputBuffer[1] = float(originalAdd.y + originalMin.y + originalMax.y + originalExchange.y);
    outputBuffer[2] = float(originalAdd4.x + originalMin4.x + originalMax4.x + originalExchange4.x);
    outputBuffer[3] = float(originalAdd4.w + originalMin4.w + originalMax4.w + originalExchange4.w);
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
