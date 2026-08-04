//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=de0d5bd1ea808c72b5b22ad5fbfc389ef05a460b63cf6e8aa1c1f87a20e14622
//META: purpose=Two equally-applicable overloads to the same call site are rejected with the "ambiguous call" diagnostic naming the function and arg types.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: 2 candidates with same-rank conversion cost from `float`
// (to `uint` and to `int8_t`) are equally applicable, producing the
// ambiguous-call diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int f(uint x) { return 1; }
int f(int8_t x) { return 2; }

void main() { int y = f(1.5); }
//CHECK:               ^ ambiguous call to 'f' with arguments of type (float)
//CHECK: E39999
