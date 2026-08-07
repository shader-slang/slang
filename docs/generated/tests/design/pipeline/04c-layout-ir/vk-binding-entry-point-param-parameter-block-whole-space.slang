//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#explicit-vkbinding-on-entry-point-parameters
//META: doc_section_digest=10008d1172c4455fc3ac51b30827a25eff107363f644dd3c90a136102fc13971
//META: purpose=A vk::binding on a whole-space entry-point parameter such as a ParameterBlock positions the register space, so every member lands in the requested set.
//META: intent=boundary
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the register-space arm of the annotation: a ParameterBlock
// consumes a whole space rather than a single descriptor slot, so the
// annotation positions the space instead of one binding. CHECK asserts both
// members of the block land in descriptor set 4 with their own sequential
// bindings, which only happens if the space itself was placed at 4.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute

struct Stuff
{
    StructuredBuffer<int> a;
    StructuredBuffer<int> b;
}

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void main([[vk::binding(0, 4)]] uniform ParameterBlock<Stuff> block)
{
    outBuf[0] = block.a[0] + block.b[0];
}

// SPV-DAG: OpDecorate %entryPointParams_block_a{{[A-Za-z0-9_]*}} DescriptorSet 4
// SPV-DAG: OpDecorate %entryPointParams_block_b{{[A-Za-z0-9_]*}} DescriptorSet 4
// SPV-DAG: OpDecorate %entryPointParams_block_a{{[A-Za-z0-9_]*}} Binding 0
// SPV-DAG: OpDecorate %entryPointParams_block_b{{[A-Za-z0-9_]*}} Binding 1
