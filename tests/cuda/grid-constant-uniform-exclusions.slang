//TEST:SIMPLE(filecheck=CHECK): -target cuda -entry computeMain -stage compute -line-directive-mode none

// Negative test for the `__grid_constant__ const` qualifier: only entry-point by-value uniform
// *aggregate* parameters get it. A scalar uniform (`uint count`) and a varying input (`uint3 tid`)
// must NOT be qualified - grid-constant is only valid on aggregates in the kernel argument space,
// so over-applying it (to a scalar or a varying) would be a CUDA compile error. Guards the two
// exclusion branches of the eligibility test.

struct Big
{
    float v[64];
}

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(
    uint3 tid: SV_DispatchThreadID,
    uniform Big big,
    uniform uint count,
    uniform RWStructuredBuffer<float> output)
{
    if (tid.x < count)
        output[tid.x] = big.v[tid.x];
}

// The whole signature is emitted on one line. The aggregate `big` is qualified; the scalar `count`
// is a plain `uint` with no qualifier immediately before it; the varying `tid` is consumed via its
// system-value semantic and is not a signature parameter at all. Matching the exact signature line
// asserts there is exactly one grid-constant qualifier, on `big` only.
// CHECK: __global__ void computeMain(SLANG_CUDA_GRID_CONSTANT const Big_0 {{[A-Za-z0-9_]+}}, uint {{[A-Za-z0-9_]+}}, RWStructuredBuffer<float>{{.*}})

// Belt and suspenders: the scalar is never preceded by the qualifier.
// CHECK-NOT: SLANG_CUDA_GRID_CONSTANT const uint
