//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#reserved-identifier-prefixes
//META: doc_section_digest=80647084abaebbe609d96190648e92969798dc148e077cfaa14fde905e6b2ed2
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
