// Q2: value_and_bwd_diff must reject void-returning functions at semantic check
// (post substitution). Use a generic instantiated to a void return.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[BackwardDifferentiable]
void noResult<T : IDifferentiable & IFloat>(T x)
{
    // Body intentionally trivial: the bug is that value_and_bwd_diff requires R != void.
}

void main()
{
    DifferentialPair<float> dp = DifferentialPair<float>(2.0, 0.0);

    // Expect a diagnostic: the wrapped function returns void.
    // CHECK: error 31232
    // CHECK: value_and_bwd_diff requires a function with a non-void return type
    value_and_bwd_diff(noResult<float>)(dp);
}
