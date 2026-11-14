//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// Negative test for the CUDA dynamic-index legalization: a parameter array that is
// only ever indexed with compile-time-constant indices resolves to static `.param`
// offsets — the fast path — and must NOT get a local copy.

// CHECK: __global__ void computeMain(FixedArray<RWStructuredBuffer<float>, 4>{{.*}}tensors_0, StructuredBuffer<uint> indices_0)
// CHECK-NOT: = tensors_0;
// CHECK: tensors_0[int(2)]

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform RWStructuredBuffer<float> tensors[4],
    uniform StructuredBuffer<uint> indices)
{
    tensors[2][tid.x] = 1.0f;
}
