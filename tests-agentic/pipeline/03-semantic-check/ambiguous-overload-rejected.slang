//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=02bdaa234074e40f6f48635687cd0c1cf99fd86fc36701fbe4f22573b9384f99
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
