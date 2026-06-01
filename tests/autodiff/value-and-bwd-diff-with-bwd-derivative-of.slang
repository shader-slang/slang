// Phase-1/phase-2 boundary: confirm that an existing user-supplied
// [BackwardDerivativeOf(f)] override on a sibling function still works when
// paired with value_and_bwd_diff(f)(...) on a different function in the same
// translation unit.

//TEST:INTERPRET(filecheck=CHECK):

// Sibling function that uses a custom-defined backward derivative.
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

void main()
{
    DifferentialPair<float> dpA = DifferentialPair<float>(7.0, 0.0);
    DifferentialPair<float> dpB = DifferentialPair<float>(3.0, 0.0);

    // The existing bwd_diff path must still pick up the user override.
    bwd_diff(scaled)(dpA, 1.0);

    // CHECK: dA=5.0
    printf("dA=%f\n", dpA.d);

    // value_and_bwd_diff in the same TU on a different function still works.
    float y = value_and_bwd_diff(square)(dpB, 1.0);

    // CHECK: y=9.0
    printf("y=%f\n", y);
    // CHECK: dB=6.0
    printf("dB=%f\n", dpB.d);
}
