//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=53cc9594629df0d6fdaba40b6892c4ce92119b3c38da01e7da3565a3cdc7634f
//META: purpose=A while loop's block body owns its own scope (the parser does not push a fresh ScopeDecl on the WhileStmt itself); a local declared in the body is not visible after the loop.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the visible boundary of a while-loop body: per the
// scope-bearing-nodes table the parser assigns no ScopeDecl to the
// WhileStmt, so the only scope isolating the body's locals is the body
// BlockStmt's own. Reading the body-local after the loop must fail as an
// undefined identifier.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int n = 0;
    while (n < 1)
    {
        int inside_while = 7;
        n = n + inside_while;
    }
    int y = inside_while;
//CHECK:    ^^^^^^^^^^^^ undefined identifier
//CHECK:    ^^^^^^^^^^^^ undefined identifier 'inside_while'.
// CHECK: E30015
}
