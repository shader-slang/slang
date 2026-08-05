//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#reserved-identifier-prefixes
//META: doc_section_digest=ff3d8b904ce316e9873091e7f19a96bdf28119cc2fa75c284e8e75440fc616b5
//META: purpose=`SV_`-prefixed identifiers are HLSL system-value semantics; `SV_Position` survives into HLSL emit as `SV_POSITION`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Tag a vertex-stage return with `SV_Position` semantic. The HLSL
// emit upper-cases the semantic to `SV_POSITION`, which is enough to
// confirm the `SV_` prefix was recognized.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage vertex

float4 main(float3 pos : POSITION) : SV_Position
{
    //CHECK: SV_POSITION
    return float4(pos, 1.0);
}
