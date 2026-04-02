//DISABLE_TEST:SIMPLE(diag=CHECK):

// Test that error 30097 should be emitted when fwd_diff is called on a
// function that is not marked as differentiable.
//
// BUG: Diagnostic 30097 is defined in slang-diagnostics.lua but never
// emitted. Instead, __fwd_diff on a non-differentiable function triggers
// an internal compiler error (E99999). This test is disabled until the
// diagnostic is properly wired up.

float plainFunc(float x)
{
    return x * x;
}

void test()
{
    __fwd_diff(plainFunc)(DifferentialPair<float>(1.0, 1.0));
//CHECK:       ^^^^^^^^^ function not differentiable
//CHECK:       ^^^^^^^^^ function 'plainFunc' is not marked as forward-differentiable.
}
