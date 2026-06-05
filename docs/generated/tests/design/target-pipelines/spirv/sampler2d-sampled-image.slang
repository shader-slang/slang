//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
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
