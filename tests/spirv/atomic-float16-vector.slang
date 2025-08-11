//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):name=workBuffer
RWStructuredBuffer<half2> workBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    half2 originalValue;
    
    // Direct atomic operation on half2 should trigger the SPV_NV_shader_atomic_fp16_vector extension
    originalValue = __atomic_add(workBuffer[0], half2(1.0h, 2.0h));
    
    outputBuffer[0] = float(originalValue.x);
    outputBuffer[1] = float(originalValue.y);
}

// CHECK: OpCapability AtomicFloat16VectorNV
// CHECK: OpExtension "SPV_NV_shader_atomic_fp16_vector"
