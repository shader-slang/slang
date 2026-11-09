//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

// Companion to overload-generic-constraint-note for a non-conformance constraint (issue #12965):
// a generic candidate rejected by a `where T == ...` equality constraint is listed with the
// general "could not satisfy the generic constraint" note (E40023) and its declaration location,
// rather than being dropped from the overload error.

struct A {}
struct B {}
struct C {}

T pick<T>(T left, T right) where T == int { return left; }
A pick(A left, A right) { return left; }
B pick(B left, B right) { return left; }

void useit(C c)
{
    // `C` matches neither concrete overload, and the generic infers `T == C`, which violates
    // `where T == int`.
    pick(c, c);
}

// One check per emitted diagnostic (SIMPLE mode is exhaustive).
//CHECK: no overload for 'pick' applicable to arguments of type (C, C)
//CHECK: candidate: func pick<T> -> T
//CHECK: could not satisfy the generic constraint 'T == int'
//CHECK: see generic constraint declaration
//CHECK: candidate: func pick(A, A) -> A
//CHECK: argument 0 does not match: expected 'A', got 'C'
//CHECK: candidate: func pick(B, B) -> B
//CHECK: argument 0 does not match: expected 'B', got 'C'
