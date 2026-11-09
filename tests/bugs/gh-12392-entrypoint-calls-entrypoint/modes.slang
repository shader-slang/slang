// Companion module for `outer-execution-mode-callee.slang`: entry points that carry
// entry-point-scoped SPIR-V execution modes other than `LocalSize`.
module modes;

[shader("compute")]
[numthreads(32, 1, 1)]
[MaximallyReconverges]
public void reconvergingKernel(uint tid, RWStructuredBuffer<float> buf)
{
    buf[tid] = tid * 3.0;
}

[shader("fragment")]
[QuadDerivatives]
public float4 quadDerivativesFragment(float4 c)
{
    return c * 2;
}

[shader("fragment")]
[RequireFullQuads]
public float4 fullQuadsFragment(float4 c)
{
    return c * 3;
}
