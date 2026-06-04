//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=mul(matrix, vector) — companion to the matrix-times-matrix case — emits OpVectorTimesMatrix because Slang's HLSL-derived mul follows the column-major convention.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: Slang follows the HLSL row-major source convention but
// SPIR-V emit uses column-major layout, so mul(matrix, vector)
// resolves to the SPIR-V OpVectorTimesMatrix opcode (not
// OpMatrixTimesVector).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<float4> outBuf;

cbuffer Params
{
    float4x4 mat;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float4 v = float4(float(tid.x), 1, 1, 1);
    outBuf[tid.x] = mul(mat, v);
}

// CHECK: OpVectorTimesMatrix
