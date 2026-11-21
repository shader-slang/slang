//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// A generic parameter inferred from several mixed-type arguments picks a
// common type only when one is strictly better, with one exception: a
// `bool` operand loses an equal-cost tie, so mixed bool/number inference
// keeps compiling (pinned in the companion tests). Every other equal-cost
// pair of distinct basic types has no common type, so inference fails
// instead of silently favoring whichever argument the solver happened to
// accumulate first.

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

    // Pointer-sized ranks are excluded from the unsigned-to-signed
    // promotion, so these pairs tie in both directions at general
    // conversion cost; neither side is `bool`, so inference fails.
    joinArgs(int64_t(0), uintptr_t(0));
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'int64_t' and 'uintptr_t'
*/
    joinArgs(uintptr_t(0), int64_t(0));
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'uintptr_t' and 'int64_t'
*/
    joinArgs(uint32_t(0), intptr_t(0));
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'uint' and 'intptr_t'
*/
    joinArgs(intptr_t(0), uint32_t(0));
/*CHECK: cannot deduce generic argument for 'T': conflicting requirements 'intptr_t' and 'uint'
*/
}
