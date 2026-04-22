// Test for issue #4107: Compiler crashes on a parameter without default value
// defined after a parameter with a default value. Should emit a diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[Differentiable]
[PreferRecompute]
float computeNEE(
    float a,
    float b = 1.0,
    no_diff float c
//CHECK:          ^ parameter 'c' does not have a default value, but follows a parameter that does
)
{
    return a + b + c;
}
