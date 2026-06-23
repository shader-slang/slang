//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=Texture2D.GetDimensions(mip, w, h, levels) dispatches via OpImageQuerySizeLod (returning v2uint) and OpImageQueryLevels; the ImageQuery capability is required.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: pins the size-query path for sampled images.
// GetDimensions on a Texture2D returns a v2uint via
// OpImageQuerySizeLod; the level-count goes through
// OpImageQueryLevels. Both depend on the ImageQuery capability.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

Texture2D<float4> tex;
RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint w, h, levels;
    tex.GetDimensions(0, w, h, levels);
    outBuf[tid.x] = w + h + levels;
}

// CHECK-DAG: OpCapability ImageQuery
// CHECK-DAG: OpImageQuerySizeLod %v2uint
// CHECK-DAG: OpImageQueryLevels
