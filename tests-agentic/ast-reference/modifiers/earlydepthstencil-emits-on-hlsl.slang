//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/llm-generated/ast-reference/modifiers.md#stage-specific-entry-point-attributes
//META: doc_section_digest=c70214a3d38c8f47b31dbc3940b51ba5fcdeb6d61435076d7a051fbb6e018f86
//META: purpose=`EarlyDepthStencilAttribute` (`[earlydepthstencil]`) on a fragment entry point survives to emitted HLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `EarlyDepthStencilAttribute` is the AST node behind the
// `[earlydepthstencil]` pixel-shader attribute. A fragment entry point
// carrying the attribute must emit it in the resulting HLSL.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -stage fragment

[earlydepthstencil]
float4 main(float4 pos : SV_Position) : SV_Target
{
    return float4(pos.x, pos.y, 0.0f, 1.0f);
}

//CHECK: [earlydepthstencil]
