//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#modifiers-vs-attributes
//META: doc_section_digest=0be0f22a51918f9fc5f00ab42a13ab6eb20120ba83f03c20f196cfda4fc6178d
//META: purpose=A `::`-qualified attribute name such as `[vk::binding(...)]` is flattened to a single identifier and resolves to the binding attribute.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the qualified attribute name is not rejected as an unknown
// two-token name but flattened into one identifier before lookup, so
// `[vk::binding(3, 2)]` finds the binding attribute. Requesting a non-default
// binding and set makes the effect visible: the CHECK-DAG lines assert the
// emitted SPIR-V decorates the variable with Binding 3 and DescriptorSet 2
// instead of the 0/0 it would receive with no attribute at all.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

[vk::binding(3, 2)]
RWStructuredBuffer<int> outBuf;

[numthreads(1,1,1)]
void main()
{
    outBuf[0] = 1;
}

//CHECK-DAG: OpDecorate %{{[A-Za-z0-9_]+}} Binding 3
//CHECK-DAG: OpDecorate %{{[A-Za-z0-9_]+}} DescriptorSet 2
