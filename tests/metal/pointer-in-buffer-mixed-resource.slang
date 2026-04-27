//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

// Cross-feature test: a ParameterBlock struct containing both a multi-level
// pointer field (lowered to ulong via the leaf pointer policy) and a
// RWStructuredBuffer<int*> (lowered via the resource-element path).
// Both lowering paths must coexist without interfering.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct Params
{
    int** indirect;
    RWStructuredBuffer<int*> ptrs;
};

ParameterBlock<Params> params;

// Multi-level pointer lowered to ulong; resource element pointers also
// lowered to ulong (via descriptor handle wrapping). No pointer-to-pointer
// types should remain in the buffer pointee struct.
// CHECK-LABEL: struct Params
// CHECK: ulong indirect
// CHECK-NOT: device{{.*}}device
// CHECK: };

// CHECK-LABEL: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int* p = params.indirect[0];
    int* q = params.ptrs[tid.x];
    p[0] = q[0];
}
