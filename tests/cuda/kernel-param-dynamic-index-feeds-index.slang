//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// Two dynamically-indexed kernel-parameter accesses where one feeds the other's
// index: `weights[idx[...]]`. Both `idx[...]` and `weights[...]` are runtime-indexed
// by-value parameter arrays, so each must be rewritten through its own local copy.
// The inner `idx` access is the index operand of the outer `weights` access, so the
// legalization must rebuild the outer access before deallocating the inner one;
// otherwise the outer access holds a dangling pointer to the freed inner GetElement.

// CHECK: __global__ void computeMain(FixedArray<float, 256>{{.*}}weights_0, FixedArray<uint, 16>{{.*}}idx_0, RWStructuredBuffer<float> output_0)
// A local copy is materialized for each of the two parameters.
// CHECK-DAG: FixedArray<float, 256>{{.*}}[[WLOCAL:_S[0-9]+]] = weights_0;
// CHECK-DAG: FixedArray<uint, 16>{{.*}}[[ILOCAL:_S[0-9]+]] = idx_0;
// The weights local copy is indexed by a value loaded from the idx local copy;
// neither parameter is indexed directly. Before the reverse-order fix, the outer
// access referenced a deallocated inner GetElement and emitted a dangling index.
// CHECK-NOT: weights_0[
// CHECK-NOT: idx_0[
// CHECK: [[WLOCAL]][{{.*}}[[ILOCAL]][

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform float weights[256],
    uniform uint idx[16],
    uniform RWStructuredBuffer<float> output)
{
    output[tid.x] = weights[idx[tid.x % 16]];
}
