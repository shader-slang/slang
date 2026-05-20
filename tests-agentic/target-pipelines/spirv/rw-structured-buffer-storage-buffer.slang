//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/target-pipelines/spirv.md#phase-d-ir-to-spir-v-emit-simplification-loop-downstream-tools
//META: doc_section_digest=701541a00a1b084515168e6adaa766b069355955c8c77e0f8307c0318b34f251
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
    outBuf[tid.x] = int(tid.x);
}

// CHECK-DAG: OpDecorate %RWStructuredBuffer Block
// CHECK-DAG: OpMemberDecorate %RWStructuredBuffer 0 Offset 0
// CHECK-DAG: OpDecorate %outBuf Binding 0
// CHECK-DAG: OpDecorate %outBuf DescriptorSet 0
// CHECK: %outBuf = OpVariable %{{[a-zA-Z_0-9]+}} StorageBuffer
