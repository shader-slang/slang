//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=981125a6c3119290dc85c7e2ef21bc96155d4ca3760c011f47ca6e6b10fd54c9
//META: purpose=Two resources receive sequential Binding 0 and Binding 1 decorations on the default DescriptorSet.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Slang's layout phase assigns sequential binding indices to
// resources declared at module scope. The first resource gets
// Binding 0, the second Binding 1, all under DescriptorSet 0.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

StructuredBuffer<int> inBuf;
RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = inBuf[tid.x];
}

// CHECK-DAG: OpDecorate %inBuf Binding 0
// CHECK-DAG: OpDecorate %outBuf Binding 1
// CHECK-DAG: OpDecorate %inBuf DescriptorSet 0
// CHECK-DAG: OpDecorate %outBuf DescriptorSet 0
