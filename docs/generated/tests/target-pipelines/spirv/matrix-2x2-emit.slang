//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
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
