//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#synthesizing-implicit-code
//META: doc_section_digest=c2e407a8f2f30473161ca1872b47d857bf827b2378eb5ccd399e2c4f1cecb0b6
//META: purpose=Initializer-list checking rejects more initializers than the synthesized constructor accepts (E30523).
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on initializer-list synthesis: a 2-field struct accepts
// 2 initializers; passing 4 is rejected with the expected/got count.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct P { int x; int y; };

void main() { P p = {1, 2, 3, 4}; }
//CHECK:            ^ too many initializers in initializer list
//CHECK:            ^ too many initializers (expected 2, got 4)
// CHECK: E30523
