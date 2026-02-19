//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

[BackwardDerivative(__d_f)]
/*CHECK:
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
    bwd_diff(f)(dpx, 1.f);
}
