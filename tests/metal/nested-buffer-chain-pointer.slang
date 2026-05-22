//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Deeply nested buffer chain: ConstantBuffer wrapping a struct that contains
// an RWStructuredBuffer whose element type itself contains a ConstantBuffer
// with a pointer field. Exercises multi-layer buffer indirection where pointers
// at the innermost level must be lowered.
//
// Requested by reviewer: https://github.com/shader-slang/slang/pull/10947

struct Inner
{
    int* ptr;
};

struct Middle
{
    RWStructuredBuffer<Inner> items;
};

ConstantBuffer<Middle> params;

RWStructuredBuffer<int> output;

// The inner struct's pointer field should be lowered to ulong inside the
// storage buffer element type.
// CHECK-NOT: device{{.*}}device

// CHECK-LABEL: computeMain

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    Inner inner = params.items[tid.x];
    output[tid.x] = *inner.ptr;
}
