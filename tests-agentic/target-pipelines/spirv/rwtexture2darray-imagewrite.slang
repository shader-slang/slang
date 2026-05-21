//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=RWTexture2DArray lowers to OpTypeImage with dim 2D, Arrayed=1, and storage-image sampled-bit 2; writes/reads go through OpImageWrite / OpImageRead.
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: arrayed + storage corner — combines Arrayed=1 with
// sampled=2 on a 2D image. Index is uint3 (xy + layer).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWTexture2DArray<float4> img;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    img[uint3(0, 0, 0)] = float4(1, 0, 0, 1);
    outBuf[tid.x] = img[uint3(0, 0, 0)];
}

// CHECK-DAG: OpTypeImage %float 2D 2 1 0 2
// CHECK-DAG: OpImageWrite
// CHECK-DAG: OpImageRead
