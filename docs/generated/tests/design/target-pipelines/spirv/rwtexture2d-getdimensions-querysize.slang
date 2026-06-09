//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=RWTexture2D.GetDimensions(w, h) dispatches via OpImageQuerySize (no Lod operand) because storage images have no mip chain; the ImageQuery capability is still required.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: storage-image size query — the SPIR-V opcode is
// OpImageQuerySize (no Lod), differing from the sampled-image
// OpImageQuerySizeLod form. Storage images don't carry mip levels.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWTexture2D<float4> img;
RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint w, h;
    img.GetDimensions(w, h);
    outBuf[tid.x] = w + h;
}

// CHECK-DAG: OpCapability ImageQuery
// CHECK-DAG: OpImageQuerySize %v2uint
