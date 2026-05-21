//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T13:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-c-spir-v-legalization-lowering-phi-elimination
//META: doc_section_digest=0621b76c92d5c66950f6bf752610276ae5f48c99c52c396b6035b11ef340daa4
//META: purpose=TextureCube.GetDimensions(mip, w, h, levels) returns a v2uint via OpImageQuerySizeLod — a cube face is square (width == height), so the result width matches a 2D image even though dim is Cube.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: Cube size query — distinct from 3D (v3uint) and
// 1D (uint). Cubemap face dimensions are returned as v2uint
// because the Cube image has two spatial dims per face.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

TextureCube<float4> tex;
RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint w, h, levels;
    tex.GetDimensions(0, w, h, levels);
    outBuf[tid.x] = w + h + levels;
}

// CHECK-DAG: OpTypeImage %float Cube
// CHECK-DAG: OpImageQuerySizeLod %v2uint
