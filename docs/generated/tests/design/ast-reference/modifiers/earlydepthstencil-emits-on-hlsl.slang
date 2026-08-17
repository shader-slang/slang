//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#stage-specific-entry-point-attributes
//META: doc_section_digest=9976dfbcc03a512351f63ab9251ef58411b90efdfa2f7539d1c820164c50911b
//META: purpose=`[earlydepthstencil]` on a fragment entry point survives to emitted HLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies an argument-less stage attribute is still carried through to the
// emitted entry point. `[earlydepthstencil]` changes the pipeline's depth-test
// ordering, so dropping it would silently change rendering; CHECK asserts the
// marker is present in the emitted HLSL.

// The same attribute is spelled per back end: GLSL turns it into an input
// layout qualifier, Metal into a function attribute, and SPIR-V into an
// execution mode on the entry point, so each prefix pins that back end's own
// form of the depth-test-ordering request.

//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry main -stage fragment
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry main -stage fragment
//TEST:SIMPLE(filecheck=METAL): -target metal -entry main -stage fragment
//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry main -stage fragment

[earlydepthstencil]
float4 main(float4 pos : SV_Position) : SV_Target
{
    return float4(pos.x, pos.y, 0.0f, 1.0f);
}

//CHECK: [earlydepthstencil]

//GLSL: layout(early_fragment_tests) in;
//METAL: {{\[\[}}early_fragment_tests{{\]\]}}
//SPIRV: OpExecutionMode %{{[A-Za-z0-9_]+}} EarlyFragmentTests
