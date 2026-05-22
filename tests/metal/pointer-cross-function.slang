//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Cross-function pointer flow: a lowered pointer value from a ParameterBlock
// is passed to a helper function. The helper receives the pointer after it has
// been reconstructed from ulong via CastIntToPtr, so the function signature
// should use a typed device pointer.
//
// Requested by reviewer: https://github.com/shader-slang/slang/pull/10947

struct Params
{
    int** indirect;
};

ParameterBlock<Params> params;

RWStructuredBuffer<int> output;

int readViaPointer(int** pp)
{
    int* p = *pp;
    return *p;
}

// The helper function should accept a typed pointer (after cast reconstruction).
// CHECK: readViaPointer
// CHECK: int device* device*

// The struct should have the multi-level pointer lowered to ulong.
// CHECK-LABEL: struct Params
// CHECK: ulong indirect
// CHECK: };

// CHECK-LABEL: computeMain

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    output[tid.x] = readViaPointer(params.indirect);
}
