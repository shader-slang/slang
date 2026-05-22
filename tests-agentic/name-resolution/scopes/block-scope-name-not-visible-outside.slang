//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=ef276a3e56c4c6414ba8d252772bf603483822a32f729a0c9feb3df3959bc61a
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
