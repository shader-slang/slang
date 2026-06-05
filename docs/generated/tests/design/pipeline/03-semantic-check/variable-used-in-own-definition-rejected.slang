//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=ba8996c1b140750bb2e807ee33d646c0a62d626b5b41e86bf40fe00640f5b270
//META: purpose=Decl checking rejects a variable whose initializer depends on the variable itself ("used in its own definition").
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Cyclic-reference boundary at the value level: a `static const`
// whose initializer references itself is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

static const int X = X + 1;
//CHECK:         ^ the initial-value expression for variable 'X' depends on the value of the variable itself

void main() { int y = X; }
