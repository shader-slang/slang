//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/name-resolution/scopes.md#scope-bearing-ast-nodes
//META: doc_section_digest=ef276a3e56c4c6414ba8d252772bf603483822a32f729a0c9feb3df3959bc61a
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
