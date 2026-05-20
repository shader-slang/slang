//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/ast-reference/base.md#declrefbase-val
//META: doc_section_digest=d2291ebf5df30d73489e8195c80420773e2998c18564f925e18ba7477ef08a77
//META: purpose=A DeclRefBase resolves a use of a name to a declaration; when no matching declaration exists in any reachable scope, the checker diagnoses the unresolved reference.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative form of DeclRefBase: an identifier that does
// not name any reachable declaration cannot be resolved to a
// DeclRefBase, and the checker emits an "undefined identifier"
// diagnostic at the unresolved-name site.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int v = doesNotExistAnywhere();
//CHECK:    ^^^^^^^^^^^^^^^^^^^^ undefined identifier
//CHECK:    ^^^^^^^^^^^^^^^^^^^^ undefined identifier 'doesNotExistAnywhere'.
}
