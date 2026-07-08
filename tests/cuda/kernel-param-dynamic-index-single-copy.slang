//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// A single by-value parameter array indexed at two different runtime indices in the
// body. Both accesses share one function-local copy (created lazily on first use and
// cached per parameter), so the prologue must emit exactly one copy-init of the
// parameter, not one per access.

// CHECK: __global__ void computeMain(FixedArray<float, 256>{{.*}}tensors_0, StructuredBuffer<uint> indices_0, RWStructuredBuffer<float> output_0)
// Exactly one prologue copy of the parameter is materialized.
// CHECK-COUNT-1: = tensors_0;
// CHECK-NOT: = tensors_0;
// CHECK-NOT: tensors_0[

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform float tensors[256],
    uniform StructuredBuffer<uint> indices,
    uniform RWStructuredBuffer<float> output)
{
    output[tid.x] = tensors[indices[tid.x]] + tensors[indices[tid.x + 1]];
}
