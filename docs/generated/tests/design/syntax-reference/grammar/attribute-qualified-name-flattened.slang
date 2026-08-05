//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#attributes-and-decorations
//META: doc_section_digest=15532574d330d6144dfcff6e6eb0a667f59639395aa4b4eccbd806a31f935008
//META: purpose=`AttributeName ::= '::'? IDENT ('::' IDENT)*` is flattened to a single identifier for lookup, so `[vk::binding(...)]` resolves.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `::`-qualified attribute name is accepted by the
// AttributeName production and resolves to a real attribute rather
// than being kept as a two-part qualified name that lookup would miss.
// If the flattening did not happen the name would not resolve and the
// explicit binding/set numbers would not appear; the SPIR-V decorations
// carrying exactly the requested 3 and 2 are the evidence it did.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain

[vk::binding(3, 2)]
RWStructuredBuffer<int> outputBuffer;

// CHECK-DAG: OpDecorate %{{[A-Za-z0-9_]+}} Binding 3
// CHECK-DAG: OpDecorate %{{[A-Za-z0-9_]+}} DescriptorSet 2
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = 1;
}
