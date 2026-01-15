// Test that pointer<->vector bitcasts use OpBitcast directly only in SPIR-V 1.5+
// Per SPIR-V spec: OpBitcast can handle pointer <-> vector<32-bit integer> bitcasts
// but only in SPIR-V 1.5 or later

//TEST:SIMPLE(filecheck=CHECK_SPIRV15): -target spirv -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_SPIRV14): -target spirv -emit-spirv-directly -profile spirv_1_4

// With SPIR-V 1.5+, pointer->vector<uint2> should use OpBitcast directly
// A 64-bit pointer can be cast to uint2 (2×32 = 64 bits)
//CHECK_SPIRV15-DAG: OpBitcast %v2uint %
//CHECK_SPIRV15-DAG: OpBitcast %_ptr_PhysicalStorageBuffer_float %

// With SPIR-V 1.4, pointer->vector<uint2> should NOT use OpBitcast directly
// Instead, it should be lowered
//CHECK_SPIRV14-NOT: OpBitcast %v2uint %.*_ptr

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=OUT):-vk -compute -emit-spirv-directly -output-using-type -capability vk_mem_model

//TEST_INPUT: set inputPtr = ubuffer(data=[1073741824 1077936128 1082130432], stride=4);
uniform float* inputPtr;

//TEST_INPUT: ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

//TEST_INPUT: ubuffer(data=[0 0], stride=4):out,name=vecBuffer
RWStructuredBuffer<uint2> vecBuffer;

// OUT: 1073741824
// OUT: 1077936128
// OUT: 0

[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // Test pointer -> vector<uint2> bitcast (should use OpBitcast in SPIR-V 1.5+)
    uint2 vec2 = bit_cast<uint2>(inputPtr);
    vecBuffer[0] = vec2;  // Force preservation
    
    // Extract the pointer value back and use it
    float* ptr_restored = bit_cast<float*>(vecBuffer[0]);
    outputBuffer[0] = asuint(ptr_restored[0]);
    outputBuffer[1] = asuint(ptr_restored[1]);
    outputBuffer[2] = vec2.x;  // Use vec2 to prevent optimization
}
