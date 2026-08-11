//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/04c-layout-ir.md#explicit-vkbinding-on-entry-point-parameters
//META: doc_section_digest=10008d1172c4455fc3ac51b30827a25eff107363f644dd3c90a136102fc13971
//META: purpose=An explicit vk::binding on an entry-point parameter is honored on the Khronos targets (SPIR-V, GLSL) and on WGSL.
//META: intent=functional
//META: pipeline_stage=layout
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the binding/set requested on an entry-point *parameter* is
// actually placed on the targets whose gate accepts it. The parameter asks
// for binding 3 in set 1 while the module-scope `outBuf` takes the default
// slot; each prefix asserts the emitted resource declaration carries 3/1
// rather than a positionally defaulted slot.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[numthreads(1, 1, 1)]
void main([[vk::binding(3, 1)]] uniform StructuredBuffer<int> inBuf)
{
    outBuf[0] = inBuf[0];
}

// SPV-DAG: OpDecorate %entryPointParams_inBuf{{[A-Za-z0-9_]*}} Binding 3
// SPV-DAG: OpDecorate %entryPointParams_inBuf{{[A-Za-z0-9_]*}} DescriptorSet 1

// GLSL-DAG: binding = 3, set = 1

// WGSL-DAG: @binding(3) @group(1)
