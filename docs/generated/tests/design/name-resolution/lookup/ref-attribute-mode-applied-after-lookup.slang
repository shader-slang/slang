//META: generated=true
//META: model=gpt-6.1-sol
//META: generated_at=2026-10-13T10:55:00Z
//META: source_commit=32438d70ef57a4f2bb4a98c8a1e3b7fcc9f47d90
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#unqualified-lookup
//META: doc_section_digest=43e51aa8f8900cb25c2253ee8c4a700f294af52f3a04c9121c8e46e771ee0e72
//META: purpose=_lookUpInScopes records the same ThisValue for both methods; checkAndAttachEffectiveThisParamInfo publishes [__ref], and checkThisExpr reads it when ConstructLookupResultExpr replays the step.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `_lookUpInScopes` records the same structural `ThisValue` breadcrumb for both references to `v`;
// it does not inspect either attribute. `checkAndAttachEffectiveThisParamInfo` recognizes `[__ref]`
// and publishes that receiver mode. `ConstructLookupResultExpr` later replays the breadcrumb, and
// `checkThisExpr` reads the checked information to make the first assignment legal. Bare `[ref]`
// is not that attribute, so it is diagnosed as unknown and the second assignment remains invalid.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute

struct Mutable
{
    int v;
    [__ref] void bump() { v = v + 1; }
}

struct NotAnAttribute
{
    int v;
    [ref] void bump() { v = v + 1; }
/*CHECK:
     ^^^ warning E31000
     ^^^ unknown attribute 'ref'
                        ^ note E30049
                          ^ error E30011
                          ^ left of '=' is not an l-value
*/
}

RWStructuredBuffer<int> gOut;

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    Mutable m;
    m.v = 1;
    m.bump();
    gOut[0] = m.v;
}
