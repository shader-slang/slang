//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// A differentiable-type-info constraint (`where __hasDiffTypeInfo(T)`) that
// fails while the generic argument is being inferred from the call must report
// the general "generic constraint not satisfied" diagnostic, rendering the
// constraint in Slang syntax as `__hasDiffTypeInfo(T)`. This is the constraint
// kind whose declaration is anonymous, so without an explicit case in the
// printer it would otherwise render as an empty string.

struct S
{
}

T f<T>(T x) where __hasDiffTypeInfo(T)
{
    return x;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    S s;
    f(s);
/*CHECK: could not satisfy the generic constraint '__hasDiffTypeInfo(T)'
*/
//CHECK: see generic constraint declaration
//CHECK-NOT: could not satisfy the generic constraint ''
}
