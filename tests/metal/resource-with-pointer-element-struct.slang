//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Struct element type containing pointers inside a resource inside a ParameterBlock.
// RWStructuredBuffer<PtrWrapper> where PtrWrapper has an int* field.
// This exercises the recursive struct-walking path in element type lowering.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct PtrWrapper
{
    int* p;
};

struct Store
{
    RWStructuredBuffer<PtrWrapper> buf;
};

ParameterBlock<Store> params;

// The struct element type should have its pointer lowered to ulong.
// CHECK-LABEL: struct PtrWrapper
// CHECK: ulong p
// CHECK: };

// The argument-buffer struct should reference the lowered element type
// without any pointer-to-pointer.
// CHECK-LABEL: struct Store
// CHECK-NOT: device{{.*}}device
// CHECK: };

// The kernel body should cast ulong back to a typed pointer.
// CHECK-LABEL: computeMain
// CHECK: int device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    PtrWrapper w = params.buf[tid.x];
    *w.p = int(tid.x);
}
