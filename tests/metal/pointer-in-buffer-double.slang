//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

// Double pointer: the slangpy failure case.
// Metal rejects pointer-to-pointer types in buffer pointee types (regardless
// of address space qualifiers). The pointer must be lowered to ulong.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct PointerArray
{
    int** ptrs;
};

ParameterBlock<PointerArray> params;

// The struct field type must not contain pointer-to-pointer.
// CHECK-LABEL: struct PointerArray
// CHECK: ulong
// CHECK-NOT: device*
// CHECK: };

// The kernel body should cast ulong back to the appropriate pointer type.
// CHECK-LABEL: computeMain
// CHECK: int device*

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    params.ptrs[tid.x][0] = int(tid.x);
}
