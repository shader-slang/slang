// Test that pointer bitcasts in Metal emit C-style casts instead of as_type<>
// Metal doesn't allow as_type<> conversions between pointers and integers.

//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

// Verify pointer-to-pointer cast uses C-style cast, not as_type.
// Allow generated symbol renaming (e.g. inputPtr -> inputPtr_0).
// CHECK: (uint device*)({{.*}}inputPtr{{.*}})

// Verify pointer-to-integer cast uses C-style cast, not as_type.
// Allow generated symbol renaming (e.g. inputPtr -> inputPtr_0).
// CHECK: (ulong)({{.*}}inputPtr{{.*}})

// Verify integer-to-pointer cast uses C-style cast, not as_type.
// Allow generated symbol renaming (e.g. ptrValue -> ptrValue_0).
// CHECK: (int device*)({{.*}}ptrValue{{.*}})

// Make sure as_type is NOT used for any pointer conversions
// CHECK-NOT: as_type<{{.*}}device{{.*}}\*>
// CHECK-NOT: as_type<{{.*}}\*>

uniform int* inputPtr;

//TEST_INPUT: ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    // Test 1: Pointer to pointer cast (int* -> uint*)
    uint* ptr2 = bit_cast<uint*>(inputPtr);

    // Test 2: Pointer to integer cast
    uint64_t ptrValue = bit_cast<uint64_t>(inputPtr);

    // Test 3: Integer to pointer cast
    int* ptr3 = bit_cast<int*>(ptrValue);

    // Use the results to prevent optimization
    outputBuffer[0] = ptr2[0];
    outputBuffer[1] = uint(ptrValue & 0xFFFFFFFF);
    outputBuffer[2] = uint(ptr3[0]);
}
