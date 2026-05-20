//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=440ef60cd49aa7b190c5138fc5695d86b4ac64644e03bf09eb6782e82ce75b0e
//META: purpose=An explicit constructor-style cast on an integer literal is converted to a target-specific cast intrinsic by the type-legalization passes.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Lower l-value cast" / value-legalization passes route a
// constructor-style `int(x)` cast through the appropriate target
// intrinsic. On HLSL the result is `int(...)` literal-style;
// on GLSL it surfaces as `int(...)`. The point of this test is
// to anchor on the doc's claim that the type-legalization
// category exists and handles cast lowering.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main

uniform float fval;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(fval);
}

// HLSL: void main
// HLSL: int(
// GLSL: void main
// GLSL: int(
