//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#swizzle-swizzleset-swizzledstore
//META: doc_section_digest=c2bc4d2909493a2c6dd101713168eaf7faf6486837f1b0abbdb93abe6ce44b5b
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
