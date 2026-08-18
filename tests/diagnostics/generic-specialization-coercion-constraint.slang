//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// A type-coercion constraint (`where int(T)`, i.e. `T` must convert to `int`)
// that fails while the generic argument is being *inferred* from the call (as
// opposed to the explicit-argument path, which has its own dedicated message)
// must report the general "generic constraint not satisfied" diagnostic,
// rendering the coercion in Slang syntax as `int(T)` with a see-declaration note.

struct S
{
}

int convert<T>(T x) where int(T)
{
    return 0;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    S s;
    convert(s);
/*CHECK: could not satisfy the generic constraint 'int(T)'
*/
//CHECK: see generic constraint declaration
}
