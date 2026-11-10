//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=9bf971e79309b04b98ac1b07e618148a41a5c833af2e6a7f18f5e4b57eddceb9
//META: purpose=A 2x2 float matrix emits OpTypeMatrix %v2float 2 — the smallest documented matrix shape on the matrix-dim boundary axis (lower edge companion to 4x4).
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: the smallest matrix dimension. The bundle pins the
// 4x4 case; this test pins the lower 2x2 edge — emits
// OpTypeMatrix %v2float 2 with a matching MatrixStride of 8.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<float2x2> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float2x2 m = float2x2(float(tid.x), 1, 0, float(tid.x));
    outBuf[tid.x] = m;
}

// CHECK-DAG: OpTypeMatrix %v2float 2
// CHECK-DAG: MatrixStride 8
