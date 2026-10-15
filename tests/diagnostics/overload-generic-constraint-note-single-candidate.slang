//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

// Scope boundary for issue #12965. When exactly one concrete overload is the single dominating
// candidate (no tie), overload resolution selects it and `CompleteOverloadCandidate` reports that
// candidate's own argument-coercion failure (E30019) — a diagnostic already targeted at the chosen
// near-miss. The constraint-failed generic reason is surfaced on the multi-candidate "no overload
// applicable" (E39999) path (see overload-generic-constraint-note.slang), not here. This test
// locks that boundary: SIMPLE mode is exhaustive, so the single E30019 annotation asserts that no
// stray E40022/E40023 constraint note is added on the single-candidate path.

interface IFoo {}
struct W {}

T g<T : IFoo>(T x) { return x; }
int g(int x) { return x; }

void use(W w)
{
    g(w);
}

//CHECK: E30019
//CHECK: expected an expression of type 'int', got 'W'
