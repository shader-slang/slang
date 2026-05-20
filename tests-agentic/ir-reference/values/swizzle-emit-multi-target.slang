//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#swizzle-swizzleset-swizzledstore
//META: doc_section_digest=43f95de1df286eb0d08e5ff850f113aedd3fa02de0fadc610d88f2bc9ba79f21
//META: purpose=A multi-component vector swizzle (`v.xyz`) lowers to a recognizable `.x` / `.y` / `.z` member-access form on every text-emit target.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The swizzle opcode is target-divergent in its emitted text:
// HLSL / GLSL / Metal / WGSL all emit `.xyz` style accessors,
// while CUDA (per the lessons-captured note on `__ldg`) factors
// uniform reads into temporaries and uses `float3 {a,b,c}` braced
// constructors. We assert each target's natural rendering
// individually rather than a shared pattern, because the doc does
// not claim cross-target uniformity for swizzle emission.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main
//TEST:SIMPLE(filecheck=METAL):-target metal -stage compute -entry main
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -stage compute -entry main
//TEST:SIMPLE(filecheck=CPP):-target cpp -stage compute -entry main

uniform float4 v;
uniform RWStructuredBuffer<float3> vbuf;

[numthreads(1, 1, 1)]
void main()
{
    vbuf[0] = v.xyz;
}

// HLSL: v_{{[0-9]+}}.xyz
// GLSL: v_{{[0-9]+}}.xyz
// METAL: v_{{[0-9]+}}.xyz
// WGSL: v_{{[0-9]+}}.xyz
// CPP: v_{{[0-9]+}}.x
// CPP-SAME: v_{{[0-9]+}}.y
// CPP-SAME: v_{{[0-9]+}}.z
