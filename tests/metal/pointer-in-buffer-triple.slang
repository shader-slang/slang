//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

// Triple pointer: ensures the fix handles arbitrary nesting depth.
// Metal rejects pointer-to-pointer types in buffer pointee types (regardless
// of address space qualifiers). The pointer must be lowered to ulong.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct DeepPointer
{
    int*** ptrs;
};

ParameterBlock<DeepPointer> params;

// The struct field type must not contain any pointer chain.
// CHECK-LABEL: struct DeepPointer
// CHECK: ulong
// CHECK-NOT: device*
// CHECK: };

// The kernel body should cast ulong back and dereference through
// the full pointer chain.
// CHECK-LABEL: computeMain
// CHECK: int device*

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    params.ptrs[0][tid.x][0] = int(tid.x);
}
