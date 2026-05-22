//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Struct element type containing pointers inside a resource inside a ParameterBlock.
// RWStructuredBuffer<PtrWrapper> where PtrWrapper has an int* field.
// Since PtrWrapper contains only single-level pointers, no lowering is needed —
// Metal accepts `device int*` inside structs in buffer-bound contexts.
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

// The struct element type keeps its pointer as a typed device pointer (no lowering).
// CHECK-LABEL: struct PtrWrapper
// CHECK: int device* p
// CHECK: };

// The argument-buffer struct should not contain pointer-to-pointer.
// CHECK-LABEL: struct Store
// CHECK-NOT: device{{.*}}device
// CHECK: };

// CHECK-LABEL: computeMain

// METALLIB: computeMain

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    PtrWrapper w = params.buf[tid.x];
    *w.p = int(tid.x);
}
