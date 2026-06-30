//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=Texture3D.GetDimensions(mip, w, h, d, levels) returns a v3uint from OpImageQuerySizeLod — the result-type width tracks the image dimensionality.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: 3D size query — OpImageQuerySizeLod result is
// v3uint (width, height, depth), distinguishing it from the
// 2D case (v2uint) and 1D case (uint).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

Texture3D<float4> tex;
RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint w, h, d, levels;
    tex.GetDimensions(0, w, h, d, levels);
    outBuf[tid.x] = w + h + d + levels;
}

// CHECK-DAG: OpTypeImage %float 3D
// CHECK-DAG: OpImageQuerySizeLod %v3uint
