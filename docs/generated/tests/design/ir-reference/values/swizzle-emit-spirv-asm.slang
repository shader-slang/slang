//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#swizzle-swizzleset-swizzledstore
//META: doc_section_digest=4d65a5f477addb92a25c5896b6cb496d8436da7a5ccfb3b367db94c8861a7ee9
//META: purpose=A multi-component vector swizzle (`v.xyz`) lowers to a recognizable `.x` / `.y` / `.z` member-access form on every text-emit target.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Closes the SPIR-V back-end of the swizzle-emission claim that the
// sibling swizzle-emit-multi-target.slang covers for HLSL/GLSL/Metal/WGSL/CPP.
// SPIR-V has no `.xyz` member-access surface syntax; the swizzle lowers
// to an OpVectorShuffle selecting components 0 1 2 from the loaded
// float4. CHECK pins that shuffle with its component selectors, leaving
// the %ids wild.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry main

uniform float4 v;
uniform RWStructuredBuffer<float3> vbuf;

[numthreads(1, 1, 1)]
void main()
{
    vbuf[0] = v.xyz;
}

// SPV: OpVectorShuffle %v3float %{{[0-9]+}} %{{[0-9]+}} 0 1 2
