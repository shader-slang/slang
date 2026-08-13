//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=7595db660b35cc02b10c1faff7c57997ed8dd2086bd9cc7a8a2f2cd14599f777
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
