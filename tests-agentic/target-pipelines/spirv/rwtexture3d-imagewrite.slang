//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=RWTexture3D lowers to an OpTypeImage with dim 3D and storage-image sampled-bit 2; legalizeImageSubscript rewrites writes/reads to OpImageWrite / OpImageRead.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: RWTexture3D — the storage-image counterpart of the
// 3D dim. Sampled bit becomes 2 (storage). Index is uint3.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWTexture3D<float4> img;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    img[uint3(tid.x, tid.y, tid.z)] = float4(1, 0, 0, 1);
    outBuf[tid.x] = img[uint3(tid.x, tid.y, tid.z)];
}

// CHECK-DAG: OpTypeImage %float 3D 2 0 0 2
// CHECK-DAG: OpImageWrite
// CHECK-DAG: OpImageRead
