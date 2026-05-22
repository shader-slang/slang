//TEST:INTERPRET(filecheck=CHECK):

// Numerical-gradient companion to mixed-fwd-bwd-out-param.slang for
// shader-slang/slang#11160. The CUDA-target sibling only verifies that
// emission no longer crashes; this test verifies the *value* of the gradient
// produced by a bwd_diff that internally calls fwd_diff on a callee with an
// `out`-differentiable parameter — the fix in `transposeMakePair` makes a
// semantic choice (primal-component = zero) and a smoke test would silently
// regress if that choice were wrong.

[Differentiable]
void doubler(float x, out float y)
{
    y = 2.0 * x;
}

// Computes df/dx of `doubler` via forward mode. For y = 2x, dy/dx = 2, so this
// returns the constant 2.0 (independent of x).
[Differentiable]
float jacOfDoubler(float x)
{
    DifferentialPair<float> dp_x = diffPair(x, 1.0);
    DifferentialPair<float> dp_y = diffPair(0.0);
    fwd_diff(doubler)(dp_x, dp_y);
    return dp_y.d;
}

// compute(x) writes 2*x + jacOfDoubler(x) * x = 2x + 2*x = 4x.
// Therefore d/dx compute = 4.
[Differentiable]
void compute(float x, out float result)
{
    float u;
    doubler(x, u);
    result = u + jacOfDoubler(x) * x;
}

void main()
{
    DifferentialPair<float> dp_x = diffPair(3.0);
    bwd_diff(compute)(dp_x, 1.0);
    // CHECK: 4.0
    printf("%f\n", dp_x.d);
}
