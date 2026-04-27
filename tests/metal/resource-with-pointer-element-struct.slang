//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

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

// The argument-buffer struct must contain ulong (lowered pointer within struct),
// not a pointer-to-pointer type.
// CHECK: struct Store
// CHECK: ulong
// CHECK-NOT: device{{.*}}device
// CHECK: };

// The kernel body should cast ulong back to a typed pointer.
// CHECK: computeMain
// CHECK: (int device*)

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    PtrWrapper w = params.buf[tid.x];
    *w.p = int(tid.x);
}
