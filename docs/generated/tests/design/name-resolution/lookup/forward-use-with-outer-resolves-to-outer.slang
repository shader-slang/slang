//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#edge-cases-and-failure-modes
//META: doc_section_digest=4c42d570b7022f6aea368273b44c89c0162efacf44581dd09dea681340a92303
//META: purpose=When a name is referenced before its DeclStmt inside a block, the inner local is hiddenFromLookup at that point; lookup falls through to an outer same-name decl rather than resolving the inner.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's failure-modes section: "Using `b` before its `DeclStmt`
// reaches the lookup with `hiddenFromLookup = true`; ... lookup
// returns either the decl from an outer scope or empty." Here the
// outer decl exists, so the forward use resolves to it — not to the
// inner local that has not yet been "declared yet" at this textual
// point.

//TEST:INTERPRET(filecheck=CHECK):

static const int q = 1000;

void main()
{
    int y = q + 1;  // outer q (1000) resolves; inner q is hidden.
    int q = 7;
    // CHECK: y=1001 inner=7
    printf("y=%d inner=%d\n", y, q);
}
