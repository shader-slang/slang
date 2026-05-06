//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test for errors 31149 (parameter type mismatch) and 31150 (signature mismatch).

// Case 1: Forward derivative with parameter type mismatch (error 31149).
[ForwardDerivative(fwd_f)]
/*CHECK:
 ^^^^^^^^^^^^^^^^^ custom derivative parameter type mismatch
 ^^^^^^^^^^^^^^^^^ invalid custom derivative. parameter type mismatch at position 0. expected 'DifferentialPair<float>', got 'int'
*/
float f(float x)
{
    return x * x;
}

DifferentialPair<float> fwd_f(int x)
{
    return diffPair(0.0, 0.0);
}

// Case 2: Backward derivative with wrong number of parameters (error 31150).
[BackwardDerivative(bwd_g)]
/*CHECK:
 ^^^^^^^^^^^^^^^^^^ custom derivative signature mismatch
 ^^^^^^^^^^^^^^^^^^ invalid custom derivative. could not resolve function with expected signature
*/
float g(float x, float y)
{
    return x + y;
}

void bwd_g(float dout)
{
}
