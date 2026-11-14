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
//
// The same direction-preservation has to hold on every shader-text target the
// claim is observable on, so the fan-out mirrors the `depth_greater` test:
// SPIR-V must say `DepthLess` (not `DepthGreater`) alongside `DepthReplacing`,
// and Metal must say `[[depth(less)]]`. Metal only honours the semantic on a
// *struct field* of the fragment return type — the return-position spelling
// silently emits `[[color(0)]]` instead (finding
// `metal-wgsl-return-position-depth-semantic-dropped`), so the METAL directive
// targets the `mainStructOut` entry point below, which carries the identical
// semantic in the shape Metal handles. WGSL has no conservative-depth
// construct and rejects `SV_DepthLessEqual` outright (E55202), so it is
// recorded as `unsupported-on-target` in the bundle README.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage fragment -entry main
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage fragment -entry main
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage fragment -entry main
//TEST:SIMPLE(filecheck=METAL):-target metal -stage fragment -entry mainStructOut

uniform float d;

float main() : SV_DepthLessEqual
{
    return d;
}

struct DepthOut
{
    float depth : SV_DepthLessEqual;
};

DepthOut mainStructOut()
{
    DepthOut o;
    o.depth = d;
    return o;
}

// GLSL-DAG: layout(depth_less) out float gl_FragDepth;

// HLSL: float main() : SV_DEPTHLESSEQUAL

// SPIRV-DAG: OpExecutionMode %main DepthReplacing
// SPIRV-DAG: OpExecutionMode %main DepthLess

// METAL: float depth_{{[0-9]+}} {{\[\[}}depth(less){{\]\]}};
