//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=Texture1D.GetDimensions returns a scalar uint via OpImageQuerySizeLod (no vector result type) — the lower edge of the size-query result-width ladder.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: 1D-image size query — the result of
// OpImageQuerySizeLod for a 1D image is a single uint, not a
// vector. Boundary axis: lowest dimensionality of the size query.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

Texture1D<float4> tex;
RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint w, levels;
    tex.GetDimensions(0, w, levels);
    outBuf[tid.x] = w + levels;
}

// CHECK-DAG: OpCapability ImageQuery
// CHECK-DAG: OpImageQuerySizeLod %uint
