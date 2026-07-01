//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#block-local-shadowing
//META: doc_section_digest=450ec1a2d3a7b9d03760ed100bf50469b13bfdf9baccdbbec743532c38c062c3
//META: purpose=Block-local shadowing hides a local from lookup until its DeclStmt is reached; using `x` before its declaration produces an undefined-identifier diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The lookup doc says a forward reference to a block-local reaches
// lookup with `hiddenFromLookup = true`, and the checker's
// undefined-identifier diagnostic takes over. Verify that consequence.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int y = x + 1;
//CHECK:    ^ undefined identifier
//CHECK:    ^ undefined identifier 'x'.
// CHECK: E30015
    int x = 5;
}
