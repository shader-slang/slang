//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=Eight distinct RWStructuredBuffer globals receive sequential Binding 0..Binding 7 decorations with DescriptorSet 0 — stresses the sequential-binding allocation past the small-count case.
//META: intent=stress
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress: many sequential resources. The bundle pins the 2-resource
// case; this exercises 8 RWStructuredBuffer parameters, all of
// which should receive ascending Binding 0..7 decorations under
// DescriptorSet 0.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> b0;
RWStructuredBuffer<int> b1;
RWStructuredBuffer<int> b2;
RWStructuredBuffer<int> b3;
RWStructuredBuffer<int> b4;
RWStructuredBuffer<int> b5;
RWStructuredBuffer<int> b6;
RWStructuredBuffer<int> b7;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    b0[tid.x] = 0;
    b1[tid.x] = 1;
    b2[tid.x] = 2;
    b3[tid.x] = 3;
    b4[tid.x] = 4;
    b5[tid.x] = 5;
    b6[tid.x] = 6;
    b7[tid.x] = 7;
}

// CHECK-DAG: OpDecorate %b0 Binding 0
// CHECK-DAG: OpDecorate %b1 Binding 1
// CHECK-DAG: OpDecorate %b2 Binding 2
// CHECK-DAG: OpDecorate %b3 Binding 3
// CHECK-DAG: OpDecorate %b4 Binding 4
// CHECK-DAG: OpDecorate %b5 Binding 5
// CHECK-DAG: OpDecorate %b6 Binding 6
// CHECK-DAG: OpDecorate %b7 Binding 7
