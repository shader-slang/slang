// A differentiable function cannot use the internal by-reference / consume
// parameter modes: they have no differentiation support, so they are rejected at
// the front end (before any autodiff IR transform runs) rather than aborting with
// an internal error deeper in the pipeline. The rejection applies regardless of
// `no_diff`, since a `no_diff`-qualified new-mode parameter would otherwise still
// reach the autodiff transforms as an unhandled direction.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry computeMain -stage compute

[ForwardDifferentiable]
float fFwd(__ref_readonly float x)
{
/*CHECK: ([[# @LINE-2]]): error 38053
*/
    return x * x;
}

[BackwardDifferentiable]
float fBwd(no_diff __consume float x, float y)
{
/*CHECK: ([[# @LINE-2]]): error 38053
*/
    return y * y;
}

[numthreads(1, 1, 1)]
void computeMain()
{
}
