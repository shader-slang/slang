//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=RWTexture1D requires the Image1D SPIR-V capability (distinct from the Sampled1D capability used for read-only Texture1D).
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: pin the storage-vs-sampled split for 1D images.
// Texture1D -> Sampled1D capability; RWTexture1D -> Image1D
// capability. Both come from processLateRequireCapabilityInsts.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWTexture1D<float4> img;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    img[tid.x] = float4(1, 0, 0, 1);
    outBuf[tid.x] = img[tid.x];
}

// CHECK: OpCapability Image1D
