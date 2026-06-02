//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=RWTexture1D lowers to an OpTypeImage with dim 1D and storage-image sampled-bit 2; legalizeImageSubscript rewrites subscript writes/reads to OpImageWrite / OpImageRead, and the Image1D capability is required.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: RWTexture1D — companion of the existing
// rw-texture-image-subscript.slang (which only covers
// RWTexture2D). 1D storage images carry the Image1D capability,
// not Sampled1D (storage vs sampled split).

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

// CHECK-DAG: OpCapability Image1D
// CHECK-DAG: OpTypeImage %float 1D 2 0 0 2
// CHECK-DAG: OpImageWrite
// CHECK-DAG: OpImageRead
