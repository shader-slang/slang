//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=float4x4 lowers to OpTypeMatrix %v4float 4 and matrix-vector multiplication emits an OpVectorTimesMatrix / OpMatrixTimesVector depending on HLSL row/column-major convention.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: largest common matrix type (4x4). The legalizeMatrixTypes
// pass keeps float4x4 as a real OpTypeMatrix; mul(m, v) lowers
// to an OpVectorTimesMatrix in SPIR-V (because Slang transposes
// the HLSL row-major matrix into the SPIR-V column-major convention).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<float4> outBuf;
StructuredBuffer<float4> inV;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float4x4 m;
    m[0] = float4(1, 0, 0, 0);
    m[1] = float4(0, 1, 0, 0);
    m[2] = float4(0, 0, 1, 0);
    m[3] = float4(0, 0, 0, 1);
    outBuf[tid.x] = mul(m, inV[tid.x]);
}

// CHECK-DAG: OpTypeMatrix %v4float 4
// CHECK: OpVectorTimesMatrix
