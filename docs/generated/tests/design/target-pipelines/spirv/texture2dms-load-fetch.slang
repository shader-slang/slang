//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=Texture2DMS.Load with a sample index dispatches via OpImageFetch with the Sample image operand instead of Lod (multi-sample images do not have mip levels).
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: the MS path of OpImageFetch. The sample-index
// operand becomes the Sample image operand, which is mutually
// exclusive with Lod (an MS image has no mip chain).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

Texture2DMS<float4> tex;
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = tex.Load(int2(0, 0), 0);
}

// CHECK-DAG: OpTypeImage %float 2D 2 0 1
// CHECK-DAG: OpImageFetch {{.*}} Sample
