//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#texture-sampler-and-texture-intrinsic-spellings
//META: doc_section_digest=bee397eb1a949a5d6416f53600e9ef89b5d2fc0ce8a121e809afa3f538c2fd12
//META: purpose=A storage texture takes a texel format and an access mode where a sampled texture takes its element type, with the format inferred from the element type by getWgslImageFormat and the mode read_write for a RWTexture.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The storage-texture branch of the `kIROp_TextureType` arm. It differs from
// the sampled branch in two independent ways, and this test pins both:
//
//   - the type parameter position holds a *texel format plus an access mode*
//     rather than an element type; and
//   - the format is inferred from the element type when the type carries no
//     `[format(...)]` attribute — `float` becomes `r32float`, `float2` becomes
//     `rg32float`, `float4` becomes `rgba32float`.
//
// All three inferred formats are exercised, because the inference is a
// per-element-type branch rather than one rule with one outcome. The access
// mode is `read_write` for every `RWTexture*`.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWTexture2D<float> rwR;
RWTexture2D<float2> rwRG;
RWTexture2D<float4> rwRGBA;
RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint2 c = tid.xy;
    outBuf[0] = rwR[c] + rwRG[c].y + rwRGBA[c].w;
}

// One inferred format per element type, each with the read_write access mode.
// CHECK-DAG: texture_storage_2d<r32float, read_write>
// CHECK-DAG: texture_storage_2d<rg32float, read_write>
// CHECK-DAG: texture_storage_2d<rgba32float, read_write>
