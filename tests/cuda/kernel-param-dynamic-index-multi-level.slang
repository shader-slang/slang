//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// A projection chain with more than one runtime index level: both levels of a
// nested array parameter are dynamically indexed. Each matched access records its
// complete chain from the parameter, so the rewrite must produce a single local
// copy of the parameter with a two-level address chain into it - not a copy per
// level, and no leftover dynamic indexing on the parameter itself.

// CHECK: __global__ void computeMain(FixedArray<FixedArray<float, 8>{{.*}}grid_0, StructuredBuffer<uint> indices_0, RWStructuredBuffer<float> output_0)
// CHECK-COUNT-1: = grid_0;
// CHECK-NOT: grid_0[
// CHECK: [[LOCAL:_S[0-9]+]][[[I:_S[0-9]+]]][[[J:_S[0-9]+]]]

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform float grid[8][8],
    uniform StructuredBuffer<uint> indices,
    uniform RWStructuredBuffer<float> output)
{
    output[tid.x] = grid[indices[tid.x]][indices[tid.x + 1]];
}
