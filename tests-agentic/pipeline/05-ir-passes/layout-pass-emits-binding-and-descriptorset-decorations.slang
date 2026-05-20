//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#layout-and-binding
//META: doc_section_digest=06fba085c49c1b3a2b0f9fcc4941f5b7e0c382078660bd5b19c43e0aa5b68350
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
