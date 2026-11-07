//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[BackwardDerivative(__d_f)]
/*CHECK:
 ^^^^^^^^^^^^^^^^^^ type mismatch in expression
 ^^^^^^^^^^^^^^^^^^ expected an expression of type 'float', got 'DifferentialPair<float>'
 ^^^^^^^^^^^^^^^^^^ custom derivative parameter type mismatch
 ^^^^^^^^^^^^^^^^^^ invalid custom derivative. parameter type mismatch at position 0. expected 'inout DifferentialPair<float>', got 'float'
*/
float f(float x)
{
    return x * x;
}

void __d_f(float x, float.Differential dout)
{
}

float main(float x)
{
    DifferentialPair<float> dpx = diffPair(x, 1.f);
    // We do not synthesize a `bwd_diff` member for `f` after rejecting its custom derivative.
    bwd_diff(f)(dpx, 1.f);
    /*CHECK:
    ^^^^^^^^ member not found
    ^^^^^^^^ 'bwd_diff' is not a member of 'f'.
    */
}
