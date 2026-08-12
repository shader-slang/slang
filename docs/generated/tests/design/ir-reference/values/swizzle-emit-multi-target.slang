//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#swizzle-swizzleset-swizzledstore
//META: doc_section_digest=b3a4828968ba320e73bd253425b6f464bf34ceea7d61ab6e1933e1f91bcb19a9
//META: purpose=A multi-component vector swizzle (`v.xyz`) lowers to a recognizable `.x` / `.y` / `.z` member-access form on every text-emit target.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a multi-component `swizzle` survives to user-visible
// text on every feasible emit target, and that each target spells it in
// its own way. HLSL, GLSL, Metal and WGSL all keep a `.xyz` accessor,
// whereas the CPU C++ backend factors the uniform read into a temporary
// and rebuilds the result as a braced `Vector<float, 3>` of three
// single-component reads. CHECK therefore pins each target's natural
// rendering separately; a shared pattern would have to be so loose it
// asserted nothing.

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
// CPP: = Vector<float, 3>
// CPP-SAME: .x,
// CPP-SAME: .y,
// CPP-SAME: .z
