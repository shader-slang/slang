//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=ef276a3e56c4c6414ba8d252772bf603483822a32f729a0c9feb3df3959bc61a
//META: purpose=Struct fields live in the AggTypeDecl's scope only; they are not visible from a free function at file scope.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// AggTypeDecl owns its own scope; fields are not in the enclosing
// file scope. A free function attempting to use `field` unqualified
// reports undefined-identifier.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S {
    int field;
    int read() { return field; }
}

void main()
{
    S s = {7};
    int x = field;
//CHECK:    ^^^^^ undefined identifier
//CHECK:    ^^^^^ undefined identifier 'field'.
}
