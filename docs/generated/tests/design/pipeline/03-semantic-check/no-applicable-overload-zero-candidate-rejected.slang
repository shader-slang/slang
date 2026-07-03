//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=de0d5bd1ea808c72b5b22ad5fbfc389ef05a460b63cf6e8aa1c1f87a20e14622
//META: purpose=A call with zero applicable candidates among multiple overloads is rejected with "no overload applicable" naming the call name.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary of N-candidate axis: 3 candidates, all requiring at
// least one argument; calling with zero args produces the
// no-applicable-overload diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int f(int x) { return 1; }
int f(int x, int y) { return 2; }
int f(int x, int y, int z) { return 3; }

void main() { int y = f(); }
//CHECK:               ^ no overload for 'f' applicable to arguments of type ()
