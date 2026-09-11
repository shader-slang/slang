//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Pins the GLSL spelling of each HLSL interpolation modifier on a fragment varying input -- `nointerpolation` becomes `flat`, `linear` becomes `smooth`, and `noperspective`/`centroid`/`sample` keep their names.
//META: intent=characterization
//META: covers=source/slang/slang-emit-glsl.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterizes `GLSLSourceEmitter::_maybeEmitInterpolationModifierText`, the table
// that converts an `IRInterpolationMode` into the GLSL qualifier that precedes a
// varying declaration. The interesting property is that the mapping is not a
// pass-through in both directions: `nointerpolation` is renamed to GLSL's `flat`
// and `linear` is renamed to GLSL's `smooth`, while `noperspective`, `centroid` and
// `sample` happen to share their spelling with HLSL. One struct field carries each
// modifier and all five are read in the return expression so none is dropped before
// emit; the CHECKs pair each emitted qualifier with the varying it belongs to, so a
// mis-mapped modifier fails rather than matching a neighbouring declaration.
//
// SPIR-V and HLSL are pinned for contrast. SPIR-V expresses the same information as
// `OpDecorate` on the input variable and, because perspective-correct centre
// interpolation is its default, emits *no* decoration for the `linear` field -- so
// the absence there is as much a part of the mapping as the four decorations that
// are present. HLSL round-trips all five modifiers verbatim.
//
// Metal and WGSL are deliberately not pinned here: both mis-map some of these five
// modifiers today, and pinning that would codify the bug. See
// `_meta/findings/metal-interpolation-modifier-linear-centroid-mismapped.yaml` and
// `_meta/findings/wgsl-interpolation-sampling-qualifier-dropped.yaml`, and the
// bundle README's `## Unreachable gaps`.
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry fragMain -stage fragment
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry fragMain -stage fragment
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry fragMain -stage fragment

struct VOut
{
    float4 pos : SV_Position;
    nointerpolation float3 flatColor : COLOR0;
    noperspective float2 npUV : TEXCOORD0;
    centroid float2 cUV : TEXCOORD1;
    sample float2 sUV : TEXCOORD2;
    linear float lin : TEXCOORD3;
};

float4 fragMain(VOut v) : SV_Target
{
    return float4(v.flatColor, 1)
        + float4(v.npUV, 0, 0)
        + float4(v.cUV, 0, 0)
        + float4(v.sUV, 0, 0)
        + v.lin;
}

// `nointerpolation` is renamed to `flat`; `linear` is renamed to `smooth`.
//GLSL-DAG: flat layout(location = {{[0-9]+}})
//GLSL-DAG: in vec3 v_flatColor
//GLSL-DAG: noperspective layout(location = {{[0-9]+}})
//GLSL-DAG: in vec2 v_npUV
//GLSL-DAG: centroid layout(location = {{[0-9]+}})
//GLSL-DAG: in vec2 v_cUV
//GLSL-DAG: sample layout(location = {{[0-9]+}})
//GLSL-DAG: in vec2 v_sUV
//GLSL-DAG: smooth layout(location = {{[0-9]+}})
//GLSL-DAG: in float v_lin

// SPIR-V decorates the input variables; perspective-correct centre interpolation is
// the default, so the `linear` input carries no interpolation decoration at all.
//SPV-DAG: OpDecorate %v_flatColor Flat
//SPV-DAG: OpDecorate %v_npUV NoPerspective
//SPV-DAG: OpDecorate %v_cUV Centroid
//SPV-DAG: OpDecorate %v_sUV Sample
//SPV-NOT: OpDecorate %v_lin Flat
//SPV-NOT: OpDecorate %v_lin NoPerspective

// HLSL reproduces the source spellings.
//HLSL-DAG: nointerpolation float3 flatColor
//HLSL-DAG: noperspective float2 npUV
//HLSL-DAG: centroid float2 cUV
//HLSL-DAG: sample float2 sUV
//HLSL-DAG: linear float lin
