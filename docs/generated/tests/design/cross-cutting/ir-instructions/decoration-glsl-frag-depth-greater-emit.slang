//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#decorations
//META: doc_section_digest=af2eb87d563403d52d71a7d33122c279b70c033339e51393dbe00f9f5f506a59
//META: purpose=A fragment output with `SV_DepthGreaterEqual` carries the `glslFragDepthGreater` decoration and drives the GLSL `layout(depth_greater)` redeclaration of `gl_FragDepth`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `glslFragDepthGreater` decoration row, which names HLSL
// `SV_DepthGreaterEqual` as the surface and the GLSL `layout(depth_greater)`
// redeclaration as the emit consequence. The GLSL CHECK asserts the
// `gl_FragDepth` redeclaration carries `depth_greater` (which is what lets a
// driver keep early-Z), and the HLSL CHECK asserts the semantic survives to
// the HLSL entry point. The depth value is read from a `uniform` so the
// constant folder cannot turn the write into a dead store the decoration
// would then have nothing to annotate.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage fragment -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage fragment -entry main

uniform float d;

float main() : SV_DepthGreaterEqual
{
    return d;
}

// GLSL-DAG: layout(depth_greater) out float gl_FragDepth;

// HLSL: float main() : SV_DEPTHGREATEREQUAL
