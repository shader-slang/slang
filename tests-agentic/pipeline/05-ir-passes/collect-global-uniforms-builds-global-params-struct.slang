//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#layout-and-binding
//META: doc_section_digest=06fba085c49c1b3a2b0f9fcc4941f5b7e0c382078660bd5b19c43e0aa5b68350
//META: purpose=The collect-global-uniforms pass packages free-floating `uniform` globals into a `GlobalParams` aggregate visible in the emit on HLSL, GLSL, and CUDA.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A bare `uniform int a;` at module scope is collected by
// `slang-ir-collect-global-uniforms.cpp` into a `GlobalParams_0`
// aggregate, surfaced as a cbuffer on HLSL, a uniform block on GLSL,
// and a `SLANG_globalParams` constant on CUDA.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a + 1;
}

// HLSL: struct GlobalParams
// HLSL: cbuffer globalParams
// GLSL: block_GlobalParams
// CUDA: SLANG_globalParams
