//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=d6a8e6a85f98f1fe1e7c4215cf8b2b2f8cf50505ce91badfb5ce987b8c7527a4
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
// CHECK: E30015
}
