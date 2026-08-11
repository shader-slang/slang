//TEST:SIMPLE(filecheck=CHECK): -target metal -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage compute -entry computeMain

// Exercises the three-pass lowering chain on a single struct:
//   1. MetalParameterBlock (no-op here — this is a ConstantBuffer, not PB)
//   2. Default: lowers float3x3 to array-of-vectors, decorates with [PhysicalType]
//   3. MetalPointerLowering: re-processes the [PhysicalType]-decorated type
//      (shouldSkipPhysicalTypes = false) to lower int** to ulong
//
// This is the only test that exercises pass 2 + pass 3 coexistence —
// a struct where the Default pass modifies some fields and the late
// pointer pass modifies others, both decorating/re-processing through
// the [PhysicalType] mechanism.
//
// See: https://github.com/shader-slang/slang/issues/10841

struct MixedFields
{
    float3x3 m;
    int** ptr;
    int tag;
};

ConstantBuffer<MixedFields> cb;
RWStructuredBuffer<int> output;

// Matrix lowered to array-of-vectors by the Default pass.
// CHECK: array<float3

// Pointer lowered to ulong by the MetalPointerLowering pass.
// CHECK: ulong ptr

// CHECK-LABEL: computeMain
// CHECK: int device* device*

// METALLIB: computeMain

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = int(cb.m[0][0]) + **cb.ptr + cb.tag;
}
