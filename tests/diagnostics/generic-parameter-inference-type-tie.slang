//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// A generic parameter inferred from several mixed-type arguments picks a
// common type only when one is strictly better. Two basic types that are
// each implicitly convertible to the other at equal cost (such as `bool`
// and `float`) have no common type, so inference fails instead of silently
// favoring whichever argument the solver happened to accumulate first.

vector<T, 3> makevec3<T>(T a, T b, T c)
{
    return vector<T, 3>(a, b, c);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // Unequal conversion costs still join regardless of argument order:
    // `uint` converts to `float` more cheaply than the reverse, so all
    // orderings of these arguments infer `T = float`.
    let a = makevec3(1.0f, 2U, 1.0f);
    let b = makevec3(2U, 1.0f, 1.0f);

    // `bool` and `float` are each implicitly convertible to the other at the
    // same cost, so no common type exists; the parameter must be stated
    // explicitly (e.g. `makevec3<float>(...)`).
    let v = makevec3(1.0f, false, 2U);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'float' and 'bool'
*/
    let u = makevec3(false, 1.0f, 2U);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'bool' and 'float'
*/
}
