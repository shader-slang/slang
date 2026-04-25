//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain

// Triple pointer variant: RWStructuredBuffer<int**> inside a ParameterBlock.
// When Metal emits this, the element type `int**` produces a triple pointer
// `device (device int* device*)*` which Metal rejects.
// The fix must handle arbitrary depth, not just double pointers.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct IndirectStore
{
    RWStructuredBuffer<int**> indirect;
};

ParameterBlock<IndirectStore> params;

// The argument-buffer struct must contain ulong (lowered pointer element),
// not a multi-level device pointer.
// CHECK: struct IndirectStore
// CHECK: ulong
// CHECK-NOT: device{{.*}}device
// CHECK: };

// The kernel body should cast ulong back to a typed pointer.
// CHECK: computeMain
// CHECK: int device*

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int** pp = params.indirect[tid.x];
    int* p = *pp;
    *p = int(tid.x);
}
