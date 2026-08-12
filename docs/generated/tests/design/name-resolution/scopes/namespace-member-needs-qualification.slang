//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=53cc9594629df0d6fdaba40b6892c4ce92119b3c38da01e7da3565a3cdc7634f
//META: purpose=A NamespaceDecl owns its own scope; members declared inside it are not visible from the enclosing module scope without qualification.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// NamespaceDecl is listed as scope-bearing. A function declared
// inside `Foo` is not in the parent's scope; using its unqualified
// name from main is undefined.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

namespace Foo {
    int helper() { return 5; }
}

void main()
{
    int x = helper();
//CHECK:    ^^^^^^ undefined identifier
//CHECK:    ^^^^^^ undefined identifier 'helper'.
// CHECK: E30015
}
