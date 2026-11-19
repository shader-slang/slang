//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive,exit-code=0):

// An equal-cost tie has no common type, so the generic parameter must be
// stated explicitly; the explicit type still accepts the mixed arguments,
// with the usual implicit-conversion warning for the `bool` argument.

vector<T, 3> makevec3<T>(T a, T b, T c)
{
    return vector<T, 3>(a, b, c);
}

void joinArgs<T>(T a, T b)
{
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    let v = makevec3<float>(1.0f, false, 2U);
/*CHECK: implicit conversion from 'bool' to 'float' is not recommended
*/

    // A vector/scalar join whose element types do not tie still succeeds:
    // the element join picks `float`, so `T` is the vector type.
    joinArgs(float3(0.0f), 2U);
}
