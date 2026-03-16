//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

// Test that a forward reference in a generic constraint
// (referencing a type parameter declared later) produces error 30117.

interface IFoo {}

// T's constraint references U, but U is declared after T.
void foo<T : U, U : IFoo>(T val)
//CHECK:     ^ forward reference in generic constraint
{
}
