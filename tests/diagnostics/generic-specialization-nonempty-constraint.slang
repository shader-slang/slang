//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// A non-empty-pack constraint (`where nonempty(T)`) that fails while the pack
// argument is being *inferred* from the call — here an empty argument list —
// must report the general "generic constraint not satisfied" diagnostic,
// rendering the constraint adaptively as `nonempty(T)` with a see-declaration
// note.

void foo<each T>(expand each T args) where nonempty(T)
{
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    foo();
/*CHECK: could not satisfy the generic constraint 'nonempty(T)'
*/
//CHECK: see generic constraint declaration
}
