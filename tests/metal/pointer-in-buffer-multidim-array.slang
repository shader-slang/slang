//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Multi-dimensional array of multi-level pointers in a ConstantBuffer.
// `int** ptrs[2][3]` is Array(Array(Ptr(Ptr(int)), 3), 2) — the pass must
// recursively preserve all array dimensions when lowering to ulong.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct Wrapper
{
    int** ptrs[2][3];
    int value;
};

ConstantBuffer<Wrapper> cb;
RWStructuredBuffer<int> output;

// The field type should be lowered to nested arrays of ulong.
// CHECK: array<ulong
// CHECK-LABEL: computeMain
// CHECK: int device* device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int** pp = cb.ptrs[0][1];
    output[tid.x] = **pp + cb.value;
}
