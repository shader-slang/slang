//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=53cc9594629df0d6fdaba40b6892c4ce92119b3c38da01e7da3565a3cdc7634f
//META: purpose=A BlockStmt introduces a fresh scope; a local declared inside is not visible outside that block.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The scope-bearing-AST-nodes table lists BlockStmt as a fresh scope.
// A local declared inside it must not be visible outside.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    {
        int local_in_block = 5;
    }
    int y = local_in_block;
//CHECK:    ^^^^^^^^^^^^^^ undefined identifier
//CHECK:    ^^^^^^^^^^^^^^ undefined identifier 'local_in_block'.
// CHECK: E30015
}
