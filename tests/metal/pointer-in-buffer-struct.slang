//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

// Metal rejects pointer-to-pointer types in buffer pointee types,
// regardless of address space qualifiers or nesting depth. The type
// itself is valid MSL — the restriction is only on buffer-bound parameters.
//
// Single-level pointer fields (int*) are preserved as typed device
// pointers since MSL accepts `device int*` in buffer-bound structs.
// Only multi-level pointers (int**, int***) need lowering to ulong.
//
// See also: pointer-in-buffer-double.slang, pointer-in-buffer-triple.slang
// See: https://github.com/shader-slang/slang/issues/10841

struct CopyParams
{
    int* src;
    int* dst;
};

ParameterBlock<CopyParams> params;

// Single-level pointers stay as typed device pointers.
// CHECK-LABEL: struct CopyParams
// CHECK: int device* src
// CHECK: int device* dst
// CHECK: };

// The kernel body accesses pointers directly (no ulong cast needed).
// CHECK-LABEL: computeMain

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    params.dst[tid.x] = params.src[tid.x];
}
