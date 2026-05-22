//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=02bdaa234074e40f6f48635687cd0c1cf99fd86fc36701fbe4f22573b9384f99
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
//CHECK: E39999
