//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/name-resolution/scopes.md#implicit-scopes
//META: doc_section_digest=104b4048e07e6c61ec2b48b83005c7517a41cbd862f9b7e13d117013974f1914
//META: purpose=A generic type parameter lives in the GenericDecl's scope; the inner decl's scope chain passes through it, so `T` resolves inside the function body.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Implicit scopes / Generic parameter list: `T` lives in the
// GenericDecl scope that wraps `identity`; from inside identity the
// parent chain reaches it.

//TEST:INTERPRET(filecheck=CHECK):

T identity<T>(T value) { return value; }

void main()
{
    int r = identity<int>(123);
    // CHECK: r=123
    printf("r=%d\n", r);
}
