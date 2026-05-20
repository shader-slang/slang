//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#layout-and-binding
//META: doc_section_digest=06fba085c49c1b3a2b0f9fcc4941f5b7e0c382078660bd5b19c43e0aa5b68350
//META: purpose=Stress: eight module-scope uniforms are packed into a single `GlobalParams` aggregate by collect-global-uniforms; every named field survives in the HLSL emit.
//META: intent=stress
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Eight uniforms exercise the many-field path of
// `slang-ir-collect-global-uniforms.cpp`. All eight names should
// appear inside the `GlobalParams_0` struct in the HLSL emit.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform int a0;
uniform int a1;
uniform int a2;
uniform int a3;
uniform int a4;
uniform int a5;
uniform int a6;
uniform int a7;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7;
}

// HLSL: struct GlobalParams
// HLSL-DAG: int a0
// HLSL-DAG: int a1
// HLSL-DAG: int a2
// HLSL-DAG: int a3
// HLSL-DAG: int a4
// HLSL-DAG: int a5
// HLSL-DAG: int a6
// HLSL-DAG: int a7
