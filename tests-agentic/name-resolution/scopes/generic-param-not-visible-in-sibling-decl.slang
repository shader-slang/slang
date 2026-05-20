//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/name-resolution/scopes.md#edge-cases-and-failure-modes
//META: doc_section_digest=984d93f961836cc85daa4d28cbdc53dade2964991d8474fd53b87f15b712e265
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
}
