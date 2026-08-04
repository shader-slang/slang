//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:46:22Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/wgsl.md#phase-b-specialization-and-type-legalization
//META: doc_section_digest=8b6c1341c5d3701b416925821dd85bff877514bbcbeea56e3e6327c0e95aa563
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
