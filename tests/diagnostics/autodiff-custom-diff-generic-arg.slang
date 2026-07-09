//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test for error 31151 (cannot resolve generic argument for derivative).
// A generic derivative whose generic parameter cannot be deduced from the original.
// Note: Using [BackwardDerivative] instead of [ForwardDerivative] because the latter
// triggers a compiler crash when generic argument resolution fails.

[BackwardDerivative(bwd_generic)]
/*CHECK:
 ^^^^^^^^^^^^^^^^^^ cannot deduce generic arguments for derivative
 ^^^^^^^^^^^^^^^^^^ The generic arguments to the derivative function cannot be deduced
*/
float original(float x)
{
    return x * x;
}

void bwd_generic<T : IDifferentiable>(inout DifferentialPair<float> x, float dout)
{
}
