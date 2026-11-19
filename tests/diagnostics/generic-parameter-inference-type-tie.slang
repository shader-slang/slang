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

void joinArgs<T>(T a, T b)
{
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    // Unequal conversion costs still join regardless of argument order:
    // `uint` converts to `float` more cheaply than the reverse, so these
    // infer `T = float`; the companion compute test asserts the inferred
    // type observably.
    let a = makevec3(1.0f, 2U, 1.0f);
    let b = makevec3(2U, 1.0f, 1.0f);

    // The third argument mirrors the issue's reproducer. It never appears
    // in the conflicts below: `uint` joins cleanly with `float` at unequal
    // cost, so the conflict is between `bool` and `float` alone.
    let v = makevec3(1.0f, false, 2U);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'float' and 'bool'
*/
    let u = makevec3(false, 1.0f, 2U);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'bool' and 'float'
*/

    // The strictly-better rule governs element types, so a join between a
    // vector and a scalar whose element types tie fails the same way.
    joinArgs(float3(0.0f), false);
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'vector<float,3>' and 'bool'
*/
    joinArgs(false, float3(0.0f));
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'bool' and 'vector<float,3>'
*/
}
