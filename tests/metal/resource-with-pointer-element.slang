//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

// Reproduces the slangpy test_copy_pointer failure:
// RWStructuredBuffer<int*> inside a struct inside a ParameterBlock generates
// `device int *device *` in the argument buffer struct, which Metal rejects
// for [[buffer(N)]]-bound parameters.
//
// This is distinct from pointer-in-buffer-{struct,double}.slang which test
// bare pointer fields. Here the pointer is the *element type* of a resource
// type (RWStructuredBuffer), and that resource lives inside the ParameterBlock.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct ValueStore
{
    RWStructuredBuffer<int*> ptrs;
};

ParameterBlock<ValueStore> params;

// The argument-buffer struct must contain ulong (lowered pointer element),
// not a multi-level device pointer.
// CHECK: struct ValueStore
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
    int* p = params.ptrs[tid.x];
    *p = int(tid.x);
}
