//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=87cc303db4e3bb3825cd2dfa57f040c4ffad9180d465ffc6172dea9a41fbd518
//META: purpose=The lower-bit-cast pass turns a Slang `bit_cast<int>(f)` into target-specific reinterpret intrinsics on HLSL and GLSL.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Lower bit cast" row of the type-and-value legalization table
// (`slang-ir-lower-bit-cast.cpp`) rewrites Slang's
// `bit_cast<T,U>(x)` into the target's intrinsic reinterpret:
// `asint` / `asuint` / `asfloat` on HLSL, `floatBitsToInt` /
// `intBitsToFloat` on GLSL.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main

uniform float f;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = bit_cast<int>(f);
}

// HLSL: asuint
// GLSL: floatBitsToInt
