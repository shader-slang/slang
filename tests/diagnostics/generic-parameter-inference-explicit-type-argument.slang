//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive,exit-code=0):

// A `bool` operand loses an equal-cost tie, so mixed bool/number inference
// joins without an explicit type; for every other equal-cost tie the
// generic parameter must still be stated explicitly (see
// generic-parameter-inference-type-tie.slang).

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
    // The explicit type still accepts the mixed arguments, with the usual
    // implicit-conversion warning for the `bool` argument.
    let v = makevec3<float>(1.0f, false, 2U);
/*CHECK: implicit conversion from 'bool' to 'float' is not recommended
*/

    // Mixed bool/number inference joins: the number side wins the tie in
    // every shape — scalar generics, builtin `?:`/`select` arms, and
    // vector/scalar and vector/vector element joins.
    let w = makevec3(1.0f, false, 2U);
    bool c = true;
    float x = select(c, 0.5f, false);
    float y = c ? 0.5f : false;
    joinArgs(float3(0.0f), false);
    joinArgs(float3(0.0f), bool3(true));

    // A vector/scalar join whose element types do not tie still succeeds:
    // the element join picks `float`, so `T` is the vector type.
    joinArgs(float3(0.0f), 2U);
}
