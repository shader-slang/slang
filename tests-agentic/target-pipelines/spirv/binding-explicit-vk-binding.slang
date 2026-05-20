//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
//META: purpose=An explicit [[vk::binding(7, 1)]] decoration places the resource on Binding 7 / DescriptorSet 1 instead of the default Binding 0 / DescriptorSet 0.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: a non-default binding index (7) and descriptor set
// (1). The boundary value is that we choose values away from
// the default 0/0 so the test catches regressions where the
// layout pass falls back to defaults.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

[[vk::binding(7, 1)]]
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// CHECK-DAG: OpDecorate %outBuf Binding 7
// CHECK-DAG: OpDecorate %outBuf DescriptorSet 1
