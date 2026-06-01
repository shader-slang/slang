// Phase-1/phase-2 boundary: an existing user-supplied [BackwardDerivativeOf(f)]
// override on one function still works (via bwd_diff) when value_and_bwd_diff is
// used on a different function in the same translation unit. value_and_bwd_diff in
// phase 1 does NOT route through such overrides — it dispatches through the
// auto-synthesized backward derivative — but the two must coexist without conflict.
// (CPU compute path — the bytecode interpreter does not support backward-diff wrappers.)

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

[BackwardDifferentiable]
float scaled(float x)
{
    return x * 5.0;
}

[BackwardDerivativeOf(scaled)]
void scaledBwd(inout DifferentialPair<float> dpx, float dOut)
{
    // d/dx (5*x) = 5
    dpx = DifferentialPair<float>(dpx.p, dpx.d + 5.0 * dOut);
}

[BackwardDifferentiable]
float square(float x)
{
    return x * x;
}

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    DifferentialPair<float> dpA = DifferentialPair<float>(7.0, 0.0);
    DifferentialPair<float> dpB = DifferentialPair<float>(3.0, 0.0);

    // The existing bwd_diff path must still pick up the user override (gradient 5).
    bwd_diff(scaled)(dpA, 1.0);
    outputBuffer[0] = dpA.d; // CHECK: 5.0

    // value_and_bwd_diff in the same TU on a different function still works.
    float y = value_and_bwd_diff(square)(dpB, 1.0);
    outputBuffer[1] = y;     // CHECK-NEXT: 9.0
    outputBuffer[2] = dpB.d; // CHECK-NEXT: 6.0
}
