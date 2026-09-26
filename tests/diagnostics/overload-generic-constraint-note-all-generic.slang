//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv

// Companion to overload-generic-constraint-note (issue #12965) for an overload set with no
// concrete candidates. Every candidate is a constraint-failed generic, so they share the worst
// status and survive into the ambiguous-candidate set rather than being pruned; the diagnostic
// must still explain the unsatisfied constraint instead of listing the candidate with no reason.

interface IA {}
interface IB {}

struct W {}

T h<T : IA>(T x) { return x; }
T h<T : IB>(T x) { return x; }

void useit(W w)
{
    // `W` conforms to neither `IA` nor `IB`.
    h(w);
}

// One check per emitted diagnostic (SIMPLE mode is exhaustive). Both overloads are listed with
// their own unsatisfied-constraint reason even though they render to the same generic signature,
// because retained candidates are deduplicated by declaration identity rather than signature.
//CHECK: no overload for 'h' applicable to arguments of type (W)
//CHECK: candidate: func h<T> -> T
//CHECK: type argument 'W' does not conform to the required interface 'IA'
//CHECK: candidate: func h<T> -> T
//CHECK: type argument 'W' does not conform to the required interface 'IB'
