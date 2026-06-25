//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=ef276a3e56c4c6414ba8d252772bf603483822a32f729a0c9feb3df3959bc61a
//META: purpose=ForStmt owns its own scope; the init-statement's variable is not visible after the loop.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// ForStmt is listed as scope-bearing (in contrast to UnscopedForStmt
// in HLSL mode). The loop variable lives in the ForStmt's scope only.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    for (int i = 0; i < 3; ++i)
    {
    }
    int z = i;
//CHECK:    ^ undefined identifier
//CHECK:    ^ undefined identifier 'i'.
// CHECK: E30015
}
