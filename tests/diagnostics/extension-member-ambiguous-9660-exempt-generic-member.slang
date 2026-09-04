//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression test for shader-slang/slang#9660: generic member functions are out of scope for the
// interim ambiguous-extension-member diagnostic (aligning two distinct generic parameter lists is
// not handled), so a generic method on the struct and a same-name generic method on an extension
// must NOT warn (41037). Exhaustive diag=CHECK fails on any unannotated diagnostic, so a clean run
// here proves no 41037 ("...declared in more than one place...") was emitted for the
// generic-member case.

struct GenHolder
{
    T gm<T>(T x) { return x; }
}

extension GenHolder
{
    T gm<T>(T x) { return x; }
}
