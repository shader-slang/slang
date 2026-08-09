//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0e5b4c31fb3cb504b87417d8391d1c84f2175e1be41f81660fa7556568fce3ac
//META: purpose=Sampler2D lowers to OpTypeSampledImage on SPIR-V because SPIR-V supports combined texture+sampler types natively; lowerCombinedTextureSamplers is skipped.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SPIR-V's OpTypeSampledImage is the native combined
// texture+sampler type. Unlike HLSL (where Slang splits Sampler2D
// into a separate texture / sampler pair), SPIR-V emit keeps the
// combined form. The pass lowerCombinedTextureSamplers is in the
// HLSL/Metal/WGSL/CPU arm only; SPIR-V is filtered out.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

Sampler2D tex;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(float2(0.5, 0.5), 0.0);
}

// CHECK: OpTypeSampledImage
