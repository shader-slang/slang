//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:13Z
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/ast-reference/values.md#modifier-values
//META: doc_section_digest=4364bf363a9056edbfcf69ad936b2e8f4e1ebe8e3f8cb1be0e64a5bc9da138b1
//META: purpose=UNormModifierVal is the type-level ModifierVal for an unorm resource-format modifier; a `Texture2D<unorm float4>` declaration preserves the unorm modifier through to the emitted HLSL surface.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `UNormModifierVal` under `ResourceFormatModifierVal`
// as "`unorm` resource format". The observable consequence is
// that the modifier is carried into the emitted HLSL spelling
// `Texture2D<unorm float4>` — i.e. the `unorm` keyword appears
// in the lowered text.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry main

[[vk::binding(0, 0)]]
Texture2D<unorm float4> tex;

[[vk::binding(1, 0)]]
RWStructuredBuffer<float4> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = tex.Load(int3(0, 0, 0));
}

// CHECK: unorm
