//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

// Companion to issue #12965: only generic candidates rejected for an unsatisfied *constraint*
// (interface conformance or a `where` clause) get a reason note. A generic rejected for a
// different inference failure must not. Here `k<T>` fails because `T` cannot be unified from the
// mismatched arguments (a unification conflict, not a constraint failure), so
// `isConstraintFailedGenericCandidate` excludes it and no E40022/E40023 note is produced. SIMPLE
// mode is exhaustive, so annotating only the E39999 error plus the two concrete-overload notes
// asserts the absence of any generic constraint note.

struct A {}
struct B {}

T k<T>(T a, T b) { return a; }
int k(int a, int b) { return a; }
float k(float a, float b) { return a; }

void use(A a, B b)
{
    k(a, b);
}

//CHECK: no overload for 'k' applicable to arguments of type (A, B)
//CHECK: candidate: func k(int, int) -> int
//CHECK: argument 0 does not match: expected 'int', got 'A'
//CHECK: candidate: func k(float, float) -> float
//CHECK: argument 0 does not match: expected 'float', got 'A'
