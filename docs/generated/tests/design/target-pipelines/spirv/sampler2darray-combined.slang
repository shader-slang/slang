//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0e5b4c31fb3cb504b87417d8391d1c84f2175e1be41f81660fa7556568fce3ac
//META: purpose=Sampler2DArray (combined image+sampler) lowers to OpTypeSampledImage backed by an OpTypeImage with dim=2D and Arrayed=1 — combining the Sampler2D combined-sampler claim with the array-image dim claim.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: corner of the combined-sampler family. The Sampler2D
// claim is pinned in the existing sampler2d-sampled-image test;
// this one adds the Arrayed axis. SPIR-V supports combined-
// sampler arrayed images natively (still no lowerCombinedTextureSamplers
// pass), so the emit stays as OpTypeSampledImage.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

Sampler2DArray tex;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(float3(0.5, 0.5, 0), 0.0);
}

// CHECK-DAG: OpTypeImage %float 2D 2 1
// CHECK-DAG: OpTypeSampledImage
