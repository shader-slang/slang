//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=d6a8e6a85f98f1fe1e7c4215cf8b2b2f8cf50505ce91badfb5ce987b8c7527a4
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
// CHECK: E30015
}
