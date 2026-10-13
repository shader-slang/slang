//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:55:00Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#unqualified-lookup
//META: doc_section_digest=43e51aa8f8900cb25c2253ee8c4a700f294af52f3a04c9121c8e46e771ee0e72
//META: purpose=_lookUpInScopes records ThisValue inside a setter; ConstructLookupResultExpr replays it, and checkThisExpr reads the receiver information published by checkAndAttachEffectiveThisParamInfo.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A setter has structural access to its receiver, so `_lookUpInScopes` records a `ThisValue` step
// for `_v`. `checkAndAttachEffectiveThisParamInfo` publishes the setter's receiver information
// during signature checking. `ConstructLookupResultExpr` replays the step, and `checkThisExpr`
// reads that information to give the receiver a writable value category. The assignment therefore
// updates the original `Holder` rather than a temporary value.

//TEST:INTERPRET(filecheck=CHECK):

struct Holder
{
    int _v;
    property int v
    {
        get { return _v; }
        set { _v = newValue * 2; }
    }
}

void main()
{
    Holder h;
    h.v = 12;
    // CHECK: r=24
    printf("r=%d\n", h._v);
}
