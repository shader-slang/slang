//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Pointer arithmetic (operator+) on the address of a multi-level pointer
// array element produces GetOffsetPtr in the IR. The pass must handle
// GetOffsetPtr the same way as GetElementPtr: update the result type to
// Ptr(ulong) and recurse into its uses to insert CastIntToPtr.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct Wrapper
{
    int** ptrs[4];
    int value;
};

ConstantBuffer<Wrapper> cb;
RWStructuredBuffer<int> output;

// CHECK-LABEL: struct Wrapper
// CHECK: ulong
// CHECK: };

// CHECK-LABEL: computeMain
// CHECK: int device* device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int** *p = __getAddress(cb.ptrs[0]);
    int** second = *(p + 1);
    output[tid.x] = **second;
}
