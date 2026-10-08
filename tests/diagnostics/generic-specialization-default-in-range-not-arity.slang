//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// A generic whose trailing parameter has a default value accepts an explicit
// argument list shorter than the full parameter count: the missing trailing
// argument is filled from the default. Here `Foo<int>` supplies one argument
// for a two-parameter generic, which is in range because `N` is defaulted.
// Specialization still fails — the default expression `T.nonexistentMember` is
// invalid — but because the supplied count is within the allowed range this
// must NOT be reported as an argument-count mismatch. It should surface the
// underlying failure and keep the general "cannot specialize" diagnostic, not a
// misleading "wrong number of generic arguments" message.

struct Foo<T, let N : int = T.nonexistentMember>
{
    T value;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    Foo<int> b;
    b.value = 0;
/*CHECK: cannot specialize generic
*/
//CHECK-NOT: wrong number of generic arguments
//CHECK-NOT: generic argument(s) but
}
