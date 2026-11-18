//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=9bf971e79309b04b98ac1b07e618148a41a5c833af2e6a7f18f5e4b57eddceb9
//META: purpose=RWStructuredBuffer<T> lowers to a Block-decorated struct in StorageBuffer storage class with Binding 0 / DescriptorSet 0 decorations.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// SPIR-V structured-buffer lowering wraps the element type in a
// struct decorated with Block, places the struct in the
// StorageBuffer storage class, and assigns default Binding 0 /
// DescriptorSet 0 decorations to the variable.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x) - 1;
}

// CHECK-DAG: OpDecorate %RWStructuredBuffer Block
// CHECK-DAG: OpMemberDecorate %RWStructuredBuffer 0 Offset 0
// CHECK-DAG: OpDecorate %outBuf Binding 0
// CHECK-DAG: OpDecorate %outBuf DescriptorSet 0
// CHECK: %outBuf = OpVariable %{{[a-zA-Z_0-9]+}} StorageBuffer
