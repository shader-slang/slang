//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:38+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/declarations.md#generics
//META: doc_section_digest=de256a399516188e43bb96d93ea3c68437fd40807db23a0da48c05221ab85f6d
//META: purpose=Implicit specialization infers generic arguments from argument types at the call site; no explicit angle-bracket arguments are needed.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says: "If a generic function/type/etc. is used where a non-generic
// function/type/etc. is expected, the compiler attempts _implicit specialization_.
// Implicit specialization infers generic arguments from the context at the use site."

//TEST:INTERPRET(filecheck=CHECK):

T myIdentityGeneric<T>(T val) { return val; }

void main()
{
    // Implicit specialization to int from argument types (no explicit <int>)
    int r = myIdentityGeneric(77);
    // CHECK: 77
    printf("%d\n", r);
}
