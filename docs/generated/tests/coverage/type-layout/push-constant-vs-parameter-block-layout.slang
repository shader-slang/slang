//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T17:00:00Z
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#global-scope-type-layout
//META: doc_section_digest=e885f1a92beacc0053db90ae24923135a05a098bfc3c282c2580a971645fcec9
//META: purpose=Verifies a vk::push_constant block lays its element type out with std430 rules in the PushConstant storage class, while a ParameterBlock uses std140 rules in the Uniform storage class on its own descriptor set.
//META: intent=characterization
//META: covers=source/slang/slang-type-layout.cpp
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the layout-rule selection in slang-type-layout.cpp for the two
// uniform-block object kinds. The same struct `S { float; float3; float }`
// is emitted twice: once via `[[vk::push_constant]] ConstantBuffer<S>` and
// once via `ParameterBlock<S>`. The push-constant path uses the std430-style
// rules (emitted type `S_std430`) and the variable lives in the PushConstant
// storage class. The ParameterBlock path uses std140 rules (emitted type
// `S_std140`) in the Uniform storage class and is allocated to its own
// descriptor set (DescriptorSet 1, distinct from the StructuredBuffer at set
// 0). The member offsets happen to coincide for this struct (0/16/28), so the
// distinguishing tokens are the type-name suffix, the storage class, and the
// descriptor-set split. Verbatim from spirv-asm emit.
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute

struct S { float a; float3 b; float c; }
[[vk::push_constant]] ConstantBuffer<S> pc;
ParameterBlock<S> pb;
RWStructuredBuffer<float> outp;
[numthreads(1, 1, 1)]
void main()
{
    outp[0] = pc.a + pc.b.x + pc.c + pb.a + pb.b.x + pb.c;
}

// Push constant: std430 element layout, PushConstant storage class.
//SPV-DAG: %pc = OpVariable %_ptr_PushConstant_S_std430 PushConstant
//SPV-DAG: OpMemberDecorate %S_std430 1 Offset 16
//SPV-DAG: OpMemberDecorate %S_std430 2 Offset 28
// ParameterBlock: std140 element layout, Uniform storage class, own set.
//SPV-DAG: OpMemberDecorate %S_std140 1 Offset 16
//SPV-DAG: OpMemberDecorate %S_std140 2 Offset 28
//SPV-DAG: OpDecorate %pb DescriptorSet 1
