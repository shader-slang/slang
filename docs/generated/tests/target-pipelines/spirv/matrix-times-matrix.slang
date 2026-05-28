//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=mul(matA, matB) for 4x4 matrices emits OpMatrixTimesMatrix — the matrix-product half of the SPIR-V matrix-arithmetic opcode family beyond primitive emission.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expansion: the doc mentions primitive type emission for vectors
// and matrices, but the SPIR-V matrix-arithmetic opcodes are not
// pinned by the bundle. mul(matA, matB) emits OpMatrixTimesMatrix.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<float4> outBuf;

cbuffer Params
{
    float4x4 matA;
    float4x4 matB;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float4x4 r = mul(matA, matB);
    outBuf[tid.x] = mul(r, float4(float(tid.x), 1, 1, 1));
}

// CHECK: OpMatrixTimesMatrix
