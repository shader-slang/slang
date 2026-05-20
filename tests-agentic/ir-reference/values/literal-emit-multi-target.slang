//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=04423d40739a5430fb339507cc0da277b4a8a29153c176db3ec3d56a42a4070b
//META: purpose=An integer literal `42` and a float literal `3.5` emit as a recognizable numeric token on every text-emit target.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Literal payloads survive lowering and emit. The doc claims
// literals are inline on the IRInst; once lowered, each backend
// emits the numeric token in its target syntax. We assert that the
// integer `42` and the float `3.5` survive into every text-emit
// target's output (regardless of any spelling differences for
// type suffixes / casts).

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CUDA):-target cuda -stage compute -entry main
//TEST:SIMPLE(filecheck=CPP):-target cpp -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;
uniform RWStructuredBuffer<float> fbuf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a + 42;
    fbuf[0] = 3.5;
}

// HLSL: 42
// GLSL: 42
// METAL: 42
// WGSL: 42
// CUDA: 42
// CPP: 42
// HLSL: 3.5
// GLSL: 3.5
// METAL: 3.5
// WGSL: 3.5
// CUDA: 3.5
// CPP: 3.5
