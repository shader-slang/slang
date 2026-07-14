//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#implicit-scopes
//META: doc_section_digest=677c751462cdb5ae5189c32a0f57ea5105199a379d69be08450135057a10b2db
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
