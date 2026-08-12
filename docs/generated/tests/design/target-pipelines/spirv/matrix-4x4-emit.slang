//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=7595db660b35cc02b10c1faff7c57997ed8dd2086bd9cc7a8a2f2cd14599f777
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
