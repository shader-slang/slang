//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=53cc9594629df0d6fdaba40b6892c4ce92119b3c38da01e7da3565a3cdc7634f
//META: purpose=A typedef declared inside a block lives in the BlockStmt's scope only; it cannot be referenced from a later statement at the enclosing scope.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// BlockStmt is a scope-bearing AST node. A typedef inside it shares
// the block's lifetime; using the alias outside the block is
// undefined.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    {
        typedef int LocalAlias;
        LocalAlias a = 1;
    }
    LocalAlias b = 2;
/*CHECK:
    ^^^^^^^^^^ undefined identifier
    ^^^^^^^^^^ undefined identifier 'LocalAlias'.
*/
// CHECK: E30015
}
