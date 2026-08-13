//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#layout-and-binding
//META: doc_section_digest=85f9eccdf732defa5a03b9b3664465b1a9ffc9a0344d31f04f8f61bca4e21c71
//META: purpose=The layout pass adds resource binding decorations; the SPIR-V emit carries explicit `Binding N` / `DescriptorSet 0` decorations for each declared resource.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `slang-ir-layout.cpp` computes IR-level layout for resources.
// The SPIR-V emit reports each resource with `OpDecorate ... Binding`
// and `OpDecorate ... DescriptorSet 0` decorations.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main

uniform RWStructuredBuffer<int> buf0;
uniform RWStructuredBuffer<int> buf1;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf0[0] = 1;
    buf1[0] = 2;
}

// SPIRV: OpDecorate {{.*}} Binding
// SPIRV: OpDecorate {{.*}} DescriptorSet
