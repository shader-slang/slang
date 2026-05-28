//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Whole-array load from a ConstantBuffer field containing multi-level pointers.
// The Load produces Array(ulong, N) which must be reconstructed element-wise
// into Array(int**, N) via CastIntToPtr before use.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct S
{
    int** arr[2];
    int tag;
};

ConstantBuffer<S> cb;
RWStructuredBuffer<int> output;

// CHECK-LABEL: struct S
// CHECK: ulong
// CHECK: };

// CHECK-LABEL: computeMain
// CHECK: int device* device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int** local[2] = cb.arr;
    output[tid.x] = **local[0];
}
