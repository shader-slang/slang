//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:55:00Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#unqualified-lookup
//META: doc_section_digest=43e51aa8f8900cb25c2253ee8c4a700f294af52f3a04c9121c8e46e771ee0e72
//META: purpose=_lookUpInScopes records the ordinary ThisValue step for a mutating method; ConstructLookupResultExpr replays it, and checkThisExpr reads the receiver information published by checkAndAttachEffectiveThisParamInfo.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `_lookUpInScopes` records only a structural `ThisValue` step for `count`; it does not inspect
// `[mutating]` or store a receiver mode. `checkAndAttachEffectiveThisParamInfo` publishes the
// method's checked receiver information during signature checking. `ConstructLookupResultExpr`
// replays the breadcrumb, and `checkThisExpr` reads that information to give the reconstructed
// receiver a writable value category. Both increments therefore update the original `Counter`.

//TEST:INTERPRET(filecheck=CHECK):

struct Counter
{
    int count;
    [mutating] void bump() { count = count + 1; }
}

void main()
{
    Counter c;
    c.count = 10;
    c.bump();
    c.bump();
    // CHECK: r=12
    printf("r=%d\n", c.count);
}
