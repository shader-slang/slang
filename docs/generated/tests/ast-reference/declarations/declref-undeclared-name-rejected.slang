//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:09:57Z
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=27f253003cd15aae74f1aea7cff6d2c4178b22d296301b6ed8e2aa0baf6c9884
//META: purpose=A reference to a name with no matching Decl in scope is diagnosed; no synthetic UnresolvedDecl reaches the user.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a name with no declaration in scope produces a
// user-facing diagnostic rather than silently resolving. This is the
// observable counterpart of "every use site refers to a Decl by
// name".

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int x = noSuchNameZzz;
//CHECK: error
//CHECK: E30015
}
