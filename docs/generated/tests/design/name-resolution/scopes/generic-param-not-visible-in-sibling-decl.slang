//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#edge-cases-and-failure-modes
//META: doc_section_digest=b57e81781278bd6752bd817f50c238aae4f56d4fda646e22befd826b34d6cab4
//META: purpose=A sibling of the outer decl that mentions a generic type parameter cannot reach it; its scope chain does not pass through the GenericDecl.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "GenericDecl parameter scope vs inner-decl scope" edge case:
// `T` is only in scope inside the generic's body. A sibling free
// function declared at file scope cannot see `T`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

T gen<T>(T x) { return x; }

void other()
{
    T y;
/*CHECK:
    ^ undefined identifier
    ^ undefined identifier 'T'.
*/
// CHECK: E30015
}
