//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#entry-point-and-parameter-handling
//META: doc_section_digest=7d2dd721ab3c1d33497a937e79d182339817290b8e4a173ec5ce69d40d28ae8d
//META: purpose=Two `uniform` globals declared at module scope are collected into the same `GlobalParams` struct by the entry-point uniforms / collect-global-uniforms passes; both fields appear in the HLSL cbuffer body.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The entry-point-uniforms pass + collect-global-uniforms together
// pack module-scope uniforms into a single aggregate that the emit
// stage renders as a cbuffer (HLSL) / uniform block (GLSL). Both
// `a` and `b` end up in the same struct.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + b;
}

// HLSL: struct GlobalParams
// HLSL: int a
// HLSL: int b
// GLSL: block_GlobalParams
// GLSL: int a
// GLSL: int b
