//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=ef276a3e56c4c6414ba8d252772bf603483822a32f729a0c9feb3df3959bc61a
//META: purpose=IfStmt itself does not own a scope, but its body parses as a BlockStmt that does; a local declared inside the then-block is not visible after the `if`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Several AST nodes do not own a fresh scope" note says IfStmt
// does not own a scope, but its body's BlockStmt does. The
// then-branch local must not escape that BlockStmt.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    bool b = true;
    if (b)
    {
        int inIf = 7;
    }
    int u = inIf;
//CHECK:    ^^^^ undefined identifier
//CHECK:    ^^^^ undefined identifier 'inIf'.
// CHECK: E30015
}
