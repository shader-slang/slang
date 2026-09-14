//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#labelstmt-breakstmt-continuestmt-and-discardstmt
//META: doc_section_digest=7c61fa1b4009ab648f41bc90151176c2c3ef804455b39f0ca18a8fb8d9a62d07
//META: purpose=A DiscardStmt is a fragment-shader pixel-kill construct; using `discard` from a non-fragment entry point is rejected as an unavailable feature for that stage.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `discard` is a fragment-only pixel-kill statement: reaching
// it from a compute entry point must be rejected. The CHECK pins the
// E36107 unavailable-feature diagnostic and the caret span the
// compiler reports on the offending statement.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry computeMain -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    discard;
//CHECK: ^^^^^^^
//CHECK: E36107
}
