//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#block-local-shadowing
//META: doc_section_digest=7314c916313330e05e1707c8796b7d952925703a5ac277526ee862347f9889d2
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
