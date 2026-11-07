//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// The CUDA dynamic-index legalization applies to plain-data parameter arrays too
// (not just resource arrays): any runtime index into `.param` space pays the
// serial load chain, so the access must be rewritten through a local copy.

// CHECK: __global__ void computeMain(FixedArray<float, 256>{{.*}}weights_0, StructuredBuffer<uint> indices_0, RWStructuredBuffer<float> output_0)
// CHECK: FixedArray<float, 256>{{.*}}[[LOCAL:_S[0-9]+]] = weights_0;
// CHECK-NOT: weights_0[
// CHECK: [[LOCAL]][

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform float weights[256],
    uniform StructuredBuffer<uint> indices,
    uniform RWStructuredBuffer<float> output)
{
    output[tid.x] = weights[indices[tid.x]];
}
