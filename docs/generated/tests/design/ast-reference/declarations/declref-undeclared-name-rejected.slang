//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/declarations.md#unresolveddecl
//META: doc_section_digest=8945d73436ca76f80fd9f167254c424a2a7da45cc3d9c7a6cb4baab6e653dd24
//META: purpose=A reference to a name with no matching declaration in scope is diagnosed rather than silently bound to a placeholder declaration.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an unbound name produces a user-facing diagnostic. The doc
// records that nothing in the tree constructs the placeholder declaration
// class, so the only observable outcome of a failed name binding is the
// E30015 error the CHECK pins.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int x = noSuchNameZzz;
//CHECK: error
//CHECK: E30015
}
