//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=c25b4949da1f02764ce1c67f1094b0f0f36902472c4e7072dce4b3577317245e
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
