// Test that pointer types use OpBitcast directly in SPIR-V where allowed by spec
// Tests pointer <-> pointer, pointer <-> integer scalar, pointer <-> integer vector

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly

// Per SPIR-V spec: pointer bitcasts MUST use direct OpBitcast when:
// - Pointer to pointer: same storage class and size
// - Pointer to integer scalar: same size
// - Integer scalar to pointer: same size
// - Pointer to integer vector (SPIR-V 1.5+): vector of 32-bit integers, same total size
// - Integer vector to pointer (SPIR-V 1.5+): vector of 32-bit integers, same total size

// Pointer to pointer (same storage class) - check that we see OpBitcast between pointers
// CHECK-DAG: OpBitcast %_ptr_PhysicalStorageBuffer_uint

// Pointer to integer scalar
// CHECK-DAG: OpBitcast %ulong %

// Integer scalar to pointer  
// CHECK-DAG: OpBitcast %_ptr_PhysicalStorageBuffer_int %

// Pointer to integer vector (SPIR-V 1.5+, only 32-bit integer vectors)
// 64-bit pointer -> uint2 (2×32 = 64 bits)
// CHECK-DAG: OpBitcast %v2uint

// Integer vector to pointer (SPIR-V 1.5+, only 32-bit integer vectors)
// uint2 -> 64-bit pointer
// CHECK-DAG: OpBitcast %_ptr_PhysicalStorageBuffer_int %

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=OUT):-vk -compute -emit-spirv-directly -output-using-type -capability vk_mem_model

//TEST_INPUT: set inputPtr = ubuffer(data=[42 100 200 300 400], stride=4);
uniform int* inputPtr;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// OUT: 42
// OUT: 100
// OUT: 200
// OUT: 42
// OUT: 42

[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // Test 1: Pointer to pointer (same storage class)
    // Cast int* to uint* (both are PhysicalStorageBuffer, same storage class)
    uint* ptr2 = bit_cast<uint*>(inputPtr);
    
    // Test 2: Pointer to integer scalar
    // Convert pointer to integer (64-bit pointer on modern systems)
    uint64_t ptrValue1 = bit_cast<uint64_t>(inputPtr);
    
    // Test 3: Integer scalar to pointer
    // Convert integer back to pointer
    int* ptr3 = bit_cast<int*>(ptrValue1);
    
    // Test 4: Pointer to integer vector (SPIR-V 1.5+, only 32-bit integer vectors)
    // 64-bit pointer -> uint2 (2×32 = 64 bits)
    uint2 ptrVec = bit_cast<uint2>(inputPtr);
    
    // Test 5: Integer vector to pointer (SPIR-V 1.5+, only 32-bit integer vectors)
    // uint2 -> 64-bit pointer
    int* ptr4 = bit_cast<int*>(ptrVec);
    
    // Verify the bitcasts worked by dereferencing the pointers
    // (They should all point to the same location)
    outputBuffer[0] = ptr2[0];
    outputBuffer[1] = ptr3[1];  // Access second element
    outputBuffer[2] = ptr4[2];  // Access third element
    
    // Verify round-trip: cast back and read original value
    outputBuffer[3] = bit_cast<uint*>(ptr3)[0];
    outputBuffer[4] = bit_cast<uint*>(ptr4)[0];
}

