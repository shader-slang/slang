//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// A generic parameter that does not appear in the primal signature cannot be inferred from a
// derivative with `[ForwardDerivativeOf]`. The checker must stop after diagnosing that failure
// instead of treating the unresolved GenericDecl as a function declaration reference.

[Differentiable]
T genericPrimal<T : IFloat, U>(T value)
{
    return value;
}

[ForwardDerivativeOf(genericPrimal)]
/*CHECK:
 ^^^^^^^^^^^^^^^^^^^ cannot deduce generic arguments for derivative
*/
DifferentialPair<float> genericPrimalForward(DifferentialPair<float> value)
{
    return value;
}
