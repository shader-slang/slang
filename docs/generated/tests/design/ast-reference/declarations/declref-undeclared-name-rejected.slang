//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#nodes
//META: doc_section_digest=1e2fffffe548d3dd55e164e2d482dadafc2362dd7c8b9d5b1675743c8d5fa05c
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
