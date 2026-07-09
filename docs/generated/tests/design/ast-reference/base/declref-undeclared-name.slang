//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/base.md#val-nodebase
//META: doc_section_digest=17754b4ad55bb3bbeb3b934dd617e4a3120a865a281a81848bf3fc5ab03948d7
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
// CHECK: E30015
}
