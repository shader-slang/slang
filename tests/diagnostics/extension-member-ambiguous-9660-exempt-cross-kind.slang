//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

// Regression test for shader-slang/slang#9660: a name shared across member categories is not a
// conflict. Two members only collide when they fall in the same category (both callables, both
// types, or both values), so a function on the struct and a nested type of the same name on an
// extension must NOT warn (41037).

struct CrossKind
{
    int sameName() { return 0; }
}

extension CrossKind
{
    struct sameName { int a; }
}

// Exhaustive diag=CHECK mode fails the test on any unannotated diagnostic, so a clean run here
// proves no 41037 ("...declared in more than one place...") was emitted for this exempt case.
