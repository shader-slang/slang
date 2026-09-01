//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#entry-point-and-parameter-handling
//META: doc_section_digest=5ecc062dc93859b69b080811db13201d5e12052d67767a4d49a4d8787cb40272
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
