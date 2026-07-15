//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#edge-cases-and-failure-modes
//META: doc_section_digest=5c32316c757a32af0aa6a61ee5f325f4830e8f73bc176d8904fdb4ddc1d8d758
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
