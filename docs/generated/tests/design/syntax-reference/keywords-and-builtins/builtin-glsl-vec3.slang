//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=3494215a2f3d95c66bfc5b7610ef9debb7242679109426ec4c710efa9fe7fa6c
//META: purpose=`vec3` is a GLSL-flavored name supplied by `glsl.meta.slang`; after `import glsl;` it resolves on every text-emit target and lowers to that target's three-component float vector.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Construct a `vec3` from runtime values and store one of its
// components to a buffer. `vec3` is contributed by `glsl.meta.slang`,
// which user code reaches with `import glsl;` (the name is not in scope
// without it; see the README doc gap). With the import, the name
// resolves on every target; each CHECK pins that target's own spelling
// of the three-component float vector.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CHECK_SPIRV):-target spirv-asm -entry main -stage compute

//CHECK: vec3 _data[]
//CHECK_HLSL: RWStructuredBuffer<float3 >
//CHECK_METAL: packed_float3 device* outBuf_0
//CHECK_WGSL: array<vec3<f32>>
//CHECK_SPIRV: OpTypeVector %float 3

import glsl;

RWStructuredBuffer<vec3> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float f = float(tid.x);
    vec3 v = vec3(f, 2.0, 3.0);
    outBuf[0] = v;
}
