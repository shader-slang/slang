//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// An unsatisfied variadic pack-count constraint (`where countof(T) == 2`) must
// be diagnosed. Supplying one pack element for a constraint that requires two
// reports the focused pack-count diagnostic naming the expected and actual
// counts, rather than a generic "cannot specialize" message.

int needsTwo<each T>(expand each T args) where countof(T) == 2
{
    return 0;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    needsTwo(1);
/*CHECK: pack count does not satisfy `countof(...)` constraint
*/
//CHECK: expected 2 elements, but pack argument has 1
//CHECK-NOT: cannot specialize generic
}
