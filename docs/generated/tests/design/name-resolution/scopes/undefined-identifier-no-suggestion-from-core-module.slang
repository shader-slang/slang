//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:25:00Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-walking-order-during-lookup
//META: doc_section_digest=3d87095dd18e90d93ff4e17cadb91ec9dacff31277081232f0c892a1279af690
//META: purpose=The close-spelling walk skips the core module, so a typo whose only near match is a builtin gets an undefined-identifier diagnostic with no "did you mean" suggestion.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the core-module exclusion in the suggestion walk: `floot` is
// one edit from the builtin `float` and nothing user-declared is close,
// so only the two undefined-identifier forms may be emitted. The test
// runs exhaustively: a spurious suggestion sourced from the core module
// would surface as an unannotated diagnostic and fail.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void skipCore()
{
    float x = floot;
//CHECK:      ^^^^^ undefined identifier
//CHECK:      ^^^^^ undefined identifier 'floot'.
// CHECK: E30015
}
