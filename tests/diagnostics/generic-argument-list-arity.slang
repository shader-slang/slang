//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// An explicit generic argument list that supplies fewer arguments than the
// generic declares under-fills the parameter list and fails specialization
// later, where the previous fallback was the generic "cannot specialize"
// message. It should now report a focused argument-count mismatch naming the
// generic and the expected/actual counts.

struct Foo<T, U>
{
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    Foo<int> b;
/*CHECK: generic 'Foo' expects 2 generic argument(s) but 1 were provided
*/
//CHECK-NOT: cannot specialize generic
}
