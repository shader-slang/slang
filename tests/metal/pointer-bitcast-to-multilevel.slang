//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Edge case: bit_cast from uint64_t to int** inside a compute kernel.
// The bit_cast result is a local multi-level pointer (not a buffer element),
// so it should NOT be lowered to ulong — it should remain as a typed pointer
// in the kernel body.
//
// Requested by reviewer: https://github.com/shader-slang/slang/pull/10947

RWStructuredBuffer<uint64_t> addrs;
RWStructuredBuffer<int> output;

// The kernel body should have a typed pointer from the bit_cast.
// CHECK-LABEL: computeMain
// CHECK: int device* device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint64_t addr = addrs[tid.x];
    int** pp = bit_cast<int**>(addr);
    int* p = *pp;
    output[tid.x] = *p;
}
