//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ast-reference/statements.md#nodes
//META: doc_section_digest=9f32f2463c3b3150d88ac333b9a732766e6f477e8e541627f6f4db0faa939a24
//META: purpose=A DiscardStmt is a fragment-shader pixel-kill construct; using `discard` from a non-fragment entry point is rejected as an unavailable feature for that stage.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc describes DiscardStmt as "discard (fragment-shader pixel
// kill)". A compute entry point exercises the rejection path: discard
// is only valid in fragment, so a compute entry point that uses it
// must diagnose that the feature is unavailable in this stage. Per the
// lessons captured the diagnostic attaches to the function signature,
// not to the `discard` token itself; we use the `note:` line to
// observe the offending token.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry computeMain -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    discard;
//CHECK: ^^^^^^^
}
