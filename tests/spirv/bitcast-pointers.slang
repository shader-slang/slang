// Test pointer bitcasts per SPIR-V spec
// Per spec: OpBitcast allows:
// - Pointer-to-pointer (same storage class)
// - Pointer-to-integer scalar (any SPIR-V version)
// - Pointer-to-integer vector (SPIR-V 1.5+)

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly

// CHECK-DAG: OpBitcast %_ptr
// CHECK-DAG: OpBitcast %ulong

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=OUT):-vk -compute -emit-spirv-directly -output-using-type

//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8 9 10], stride=4):name=inputBuffer
RWStructuredBuffer<uint> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// OUT: 1
// OUT: 2
// OUT: 3
// OUT: 4

[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // Test 1: Pointer to pointer of different type (same storage class)
    // This tests ptr<uint> -> ptr<int> bitcast
    uint* ptrUint = &inputBuffer[0];
    int* ptrInt = bit_cast<int*>(ptrUint);
    outputBuffer[0] = uint(*ptrInt);

    // Test 2: Pointer to pointer with different pointee type
    uint* ptrUint2 = &inputBuffer[1];
    float* ptrFloat = bit_cast<float*>(ptrUint2);
    outputBuffer[1] = asuint(*ptrFloat);

    // Test 3: Pointer to integer (64-bit pointer to uint64_t)
    uint* ptrUint3 = &inputBuffer[2];
    uint64_t ptrAsInt = bit_cast<uint64_t>(ptrUint3);
    uint* ptrRestored = bit_cast<uint*>(ptrAsInt);
    outputBuffer[2] = *ptrRestored;

    // Test 4: Another pointer-to-pointer conversion
    uint* ptrUint4 = &inputBuffer[3];
    uint16_t* ptrUint16 = bit_cast<uint16_t*>(ptrUint4);
    outputBuffer[3] = uint(*ptrUint16);
}
