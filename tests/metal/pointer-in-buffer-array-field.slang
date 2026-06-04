//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Array of multi-level pointers as a struct field in a ConstantBuffer.
// Metal's rejection is transitive through array element types:
// `int** arr[2]` produces `array<device int* device*, 2>` which is invalid.
// The pass must detect multi-level pointers inside array element types.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct Wrapper
{
    int** ptrs[2];
    int value;
};

ConstantBuffer<Wrapper> cb;
RWStructuredBuffer<int> output;

// The array element type should be lowered to ulong.
// CHECK: array<ulong
// CHECK-LABEL: computeMain
// CHECK: int device* device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int** pp = cb.ptrs[0];
    output[tid.x] = **pp;
}
