//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/ir-instructions.md#decorations
//META: doc_section_digest=af2eb87d563403d52d71a7d33122c279b70c033339e51393dbe00f9f5f506a59
//META: purpose=A fragment output with `SV_DepthLessEqual` carries the `glslFragDepthLess` decoration and drives the GLSL `layout(depth_less)` redeclaration of `gl_FragDepth`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second half of the `glslFragDepthGreater` /
// `glslFragDepthLess` decoration row: the two decorations are distinct
// opcodes with opposite meanings, so the only way to show the mapping is
// direction-preserving is to compile the `SV_DepthLessEqual` surface and
// assert the GLSL redeclaration says `depth_less` (not `depth_greater`).
// The HLSL CHECK pins the semantic on the emitted entry point for the same
// reason.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage fragment -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage fragment -entry main

uniform float d;

float main() : SV_DepthLessEqual
{
    return d;
}

// GLSL-DAG: layout(depth_less) out float gl_FragDepth;

// HLSL: float main() : SV_DEPTHLESSEQUAL
