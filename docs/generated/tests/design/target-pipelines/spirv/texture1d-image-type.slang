//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=Texture1D lowers to OpTypeImage with the 1D dimension and triggers the Sampled1D capability.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: Texture1D pins the 1D-dim row of the SPIR-V image
// table that the existing bundle does not cover. The 1D form
// also requires the Sampled1D capability under Vulkan, which is
// part of the resolveTextureFormat / image-type lowering.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

Texture1D<float4> tex;
SamplerState samp;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.SampleLevel(samp, 0.5, 0.0);
}

// CHECK-DAG: OpCapability Sampled1D
// CHECK-DAG: OpTypeImage %float 1D
