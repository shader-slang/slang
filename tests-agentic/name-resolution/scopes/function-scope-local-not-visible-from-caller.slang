//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=ef276a3e56c4c6414ba8d252772bf603483822a32f729a0c9feb3df3959bc61a
//META: purpose=A FuncDecl owns its own scope; a local declared inside helper() is not visible from another function's scope.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// CallableDecl/FuncDecl is listed as scope-bearing. A name introduced
// in helper's body must not be reachable from main's body.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void helper()
{
    int local_in_helper = 5;
}

void main()
{
    helper();
    int y = local_in_helper;
//CHECK:    ^^^^^^^^^^^^^^^ undefined identifier
//CHECK:    ^^^^^^^^^^^^^^^ undefined identifier 'local_in_helper'.
}
