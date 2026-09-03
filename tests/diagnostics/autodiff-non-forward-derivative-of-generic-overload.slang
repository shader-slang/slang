//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Overloaded generic primals are not yet supported by `[BackwardDerivativeOf]` or
// `[PrimalSubstituteOf]`. Both paths must report a source diagnostic instead of treating the
// unresolved overload selection as a function declaration reference.

__generic<T : __BuiltinFloatingPointType, let N : int>
[Differentiable]
vector<T, N> backwardPrimal(vector<T, N> value)
{
    return value * value;
}

[Differentiable]
T backwardPrimal<T : IFloat>(T value)
{
    return value * value;
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[BackwardDerivativeOf(backwardPrimal)]
/*CHECK:
 ^^^^^^^^^^^^^^^^^^^^ cannot resolve derivative function
 ^^^^^^^^^^^^^^^^^^^^ cannot resolve the custom derivative function
                      ^^^^^^^^^^^^^^ deprecated generic-parameter-count overload tie-breaker
                      ^^^^^^^^^^^^^^ overload resolution selected an otherwise ambiguous candidate by preferring the single candidate with the fewest required generic parameters; this tie-breaker is removed in Slang 202c
*/
void backwardPrimalDerivative(
    inout DifferentialPair<vector<T, N>> value,
    vector<T, N> resultDifferential)
{
    updateDiff(value, T(2.0) * value.p * resultDifferential);
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[Differentiable]
vector<T, N> substitutePrimal(vector<T, N> value)
{
    return value * value;
}

[Differentiable]
T substitutePrimal<T : IFloat>(T value)
{
    return value * value;
}

__generic<T : __BuiltinFloatingPointType, let N : int>
[PrimalSubstituteOf(substitutePrimal)]
/*CHECK:
 ^^^^^^^^^^^^^^^^^^ cannot resolve derivative function
 ^^^^^^^^^^^^^^^^^^ cannot resolve the custom derivative function
                    ^^^^^^^^^^^^^^^^ deprecated generic-parameter-count overload tie-breaker
                    ^^^^^^^^^^^^^^^^ overload resolution selected an otherwise ambiguous candidate by preferring the single candidate with the fewest required generic parameters; this tie-breaker is removed in Slang 202c
*/
[Differentiable]
vector<T, N> substitutePrimalReplacement(vector<T, N> value)
{
    return value;
}
