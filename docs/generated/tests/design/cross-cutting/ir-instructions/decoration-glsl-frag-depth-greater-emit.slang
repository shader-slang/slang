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
//
// The claim is a codegen claim, so it is fanned out across the shader-text
// targets. SPIR-V spells the same conservative-depth promise as the
// `DepthGreater` execution mode (paired with `DepthReplacing`, which is what
// says the shader writes depth at all), and Metal spells it as the
// `[[depth(greater)]]` attribute on the depth output. Metal only honours the
// semantic when it sits on a *struct field* of the fragment return type — the
// return-position spelling `float main() : SV_DepthGreaterEqual` silently
// emits `[[color(0)]]` instead (finding
// `metal-wgsl-return-position-depth-semantic-dropped`), so `mainStructOut`
// below carries the same semantic in the shape Metal does handle, and the
// METAL directive targets that entry point. WGSL has no conservative-depth
// construct at all and rejects the semantic outright (E55202), so it is
// recorded as `unsupported-on-target` in the bundle README rather than given
// a weakened directive.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage fragment -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage fragment -entry main
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage fragment -entry main
//TEST:SIMPLE(filecheck=METAL):-target metal -stage fragment -entry mainStructOut

uniform float d;

float main() : SV_DepthGreaterEqual
{
    return d;
}

struct DepthOut
{
    float depth : SV_DepthGreaterEqual;
};

DepthOut mainStructOut()
{
    DepthOut o;
    o.depth = d;
    return o;
}

// GLSL-DAG: layout(depth_greater) out float gl_FragDepth;

// HLSL: float main() : SV_DEPTHGREATEREQUAL

// SPIRV-DAG: OpExecutionMode %main DepthReplacing
// SPIRV-DAG: OpExecutionMode %main DepthGreater

// METAL: float depth_{{[0-9]+}} {{\[\[}}depth(greater){{\]\]}};
