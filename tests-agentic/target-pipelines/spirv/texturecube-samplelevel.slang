//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=TextureCube.SampleLevel dispatches via OpImageSampleExplicitLod with Lod on a Cube OpTypeImage, using a 3-component direction vector.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: the Cube-dim sampled-image dispatch — the existing
// -sampled-image.slang test only pins OpTypeImage; this one
// pins the SampleLevel lowering through OpImageSampleExplicitLod.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

TextureCube<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, float3(1, 0, 0), 0.0);
}

// CHECK-DAG: OpTypeImage %float Cube
// CHECK-DAG: OpImageSampleExplicitLod
