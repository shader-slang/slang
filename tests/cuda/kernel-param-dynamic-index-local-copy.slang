//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// A CUDA kernel parameter lives in `.param` (kernel-argument) space, which ptxas
// cannot address dynamically: a runtime index into a by-value parameter array is
// lowered to a serial per-element load chain (issue #11774). Verify that the
// dynamic-index legalization pass materializes a function-local copy of the
// parameter in the kernel prologue and indexes the copy instead, and that the
// kernel signature (the host-visible ABI) is unchanged.

// The signature keeps the by-value array parameter (no ABI change).
// CHECK: __global__ void computeMain(FixedArray<RWStructuredBuffer<float>, 32>{{.*}}tensors_0, StructuredBuffer<uint> indices_0)

// The prologue copies the parameter into a local, and the runtime index goes
// through the local copy, not the parameter.
// CHECK: FixedArray<RWStructuredBuffer<float>, 32>{{.*}}[[LOCAL:_S[0-9]+]] = tensors_0;
// CHECK-NOT: tensors_0[
// CHECK: [[LOCAL]][

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform RWStructuredBuffer<float> tensors[32],
    uniform StructuredBuffer<uint> indices)
{
    tensors[indices[tid.x]][tid.x] = 1.0f;
}
