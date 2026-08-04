//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#synthesizing-implicit-code
//META: doc_section_digest=51cb62c0b32d5853269f2948b06d5fe885de6c3a839dfdb3e56f3ab895330c38
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
