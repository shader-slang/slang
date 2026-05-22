//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// ParameterBlock containing a struct with an RWStructuredBuffer<int*> field,
// exercising the resource-element pointer lowering path in argument buffer
// structs. The buffer's pointer element type must be lowered to ulong to
// avoid pointer-to-pointer in the argument buffer struct.
//
// Requested by reviewer: https://github.com/shader-slang/slang/pull/10947

struct Params
{
    RWStructuredBuffer<int*> ptrs;
};

ParameterBlock<Params> params;

RWStructuredBuffer<int> output;

// The argument-buffer struct must not contain pointer-to-pointer types.
// CHECK-NOT: device{{.*}}device

// The kernel body should cast back to a typed pointer.
// CHECK-LABEL: computeMain
// CHECK: (int device*)

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int* p = params.ptrs[tid.x];
    output[tid.x] = *p;
}
