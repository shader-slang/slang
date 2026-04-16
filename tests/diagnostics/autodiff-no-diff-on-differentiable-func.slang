//DISABLE_DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Test that error 38032 should be emitted when no_diff is applied to a
// call to a differentiable function inside a differentiable function.
//
// BUG: Diagnostic 38032 is defined in slang-diagnostics.lua but never
// emitted. Using no_diff on a differentiable function call silently
// succeeds. This test is disabled until the diagnostic is properly
// wired up.

[Differentiable]
float g(float x)
{
    return x * x;
}

[Differentiable]
float f(float x)
{
    return no_diff g(x);
//CHECK:   ^^^^^^^ 'no_diff' on differentiable function has no meaning
//CHECK:   ^^^^^^^ use 'no_diff' on a call to a differentiable function has no meaning.
}
