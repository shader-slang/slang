//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=03a91c30e9eaf1766fcdfbae6c7316ecb8108a2f9e0a27218a3bbcea152c7a7f
//META: purpose=lowerCombinedTextureSamplers splits Sampler2D into a separate texture and sampler pair on WGSL
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// WGSL has no combined texture+sampler type; the pass introduces
// <name>_texture_* and <name>_sampler_* globals.

//TEST:SIMPLE(filecheck=CHECK):-target wgsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;
Sampler2D combinedTex;

[shader("compute")]
[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = combinedTex.SampleLevel(float2(0, 0), 0).x;
}

// CHECK: combinedTex_texture_{{[0-9]+}}
// CHECK-SAME: texture_2d
// CHECK: combinedTex_sampler_{{[0-9]+}}
// CHECK-SAME: sampler
