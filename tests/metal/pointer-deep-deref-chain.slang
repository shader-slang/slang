//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Multi-level dereferencing at different access chain depths: a triple pointer
// field is dereferenced step by step. Each level should produce a correct cast
// from ulong back to a typed pointer.
//
// Requested by reviewer: https://github.com/shader-slang/slang/pull/10947

struct Params
{
    int*** deep;
};

ParameterBlock<Params> params;

RWStructuredBuffer<int> output;

// The struct should have the triple pointer lowered to ulong.
// CHECK-LABEL: struct Params
// CHECK: ulong deep
// CHECK: };

// The kernel body should reconstruct typed pointers at each dereference level.
// CHECK-LABEL: computeMain

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    int*** ppp = params.deep;
    int** pp = *ppp;
    int* p = *pp;
    int val = *p;
    output[tid.x] = val;
}
