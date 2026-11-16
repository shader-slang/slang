//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/scopes.md#implicit-scopes
//META: doc_section_digest=19ad073ab979d4a331ac1f5fc84f58af039847fad8e90dbe9db085b55e74e2b0
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
