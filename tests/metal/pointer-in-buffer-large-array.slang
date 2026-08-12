//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Regression test for loop-based pack/unpack of large pointer arrays.
// When a struct contains >kMaxArraySizeToUnroll (32) pointer elements,
// lowerBufferElementTypeToStorageType generates loop-based helpers via
// emitLoopBlocks, which introduce block parameters (phis). These must
// be eliminated before emission since the main eliminatePhis already ran.
//
// Without the late eliminatePhis, the loop counter is emitted as an
// undeclared variable and the generated Metal code fails to compile.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct BigPtrArray
{
    int** ptrs[33];
    int tag;
};

BigPtrArray cloneBigPtrArray(BigPtrArray src)
{
    return src;
}

ConstantBuffer<BigPtrArray> cb;
RWStructuredBuffer<int> output;

// The storage struct should lower pointer fields to ulong.
// CHECK: array<ulong

// The loop-based unpack should have a properly declared loop counter
// and reconstruct typed pointers from the lowered ulong values.
// CHECK-LABEL: cloneBigPtrArray
// CHECK: int(0)
// CHECK: < int(33)
// CHECK: int device* device*

// CHECK-LABEL: computeMain

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    BigPtrArray local = cloneBigPtrArray(cb);
    output[tid.x] = **local.ptrs[0] + **local.ptrs[32] + local.tag;
}
