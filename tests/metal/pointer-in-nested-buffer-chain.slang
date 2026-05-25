//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Deeply nested buffer chain: ConstantBuffer wrapping a struct that contains
// an RWStructuredBuffer whose element type is a struct with a single-level
// pointer field. Single-level pointers (int*) are valid Metal and should NOT
// be lowered — this test verifies they are preserved as typed device pointers.
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

// The inner struct's single-level pointer field should be preserved as a
// typed device pointer (not lowered to ulong).
// CHECK: int device* ptr
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
