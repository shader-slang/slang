//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Characterizes the GLSL mapping of the fragment coverage and depth system values, where SV_Coverage becomes element 0 of gl_SampleMaskIn/gl_SampleMask, SV_InnerCoverage becomes gl_FragFullyCoveredNV and SV_DepthGreaterEqual becomes a layout(depth_greater) gl_FragDepth.
//META: intent=characterization
//META: covers=source/slang/slang-ir-glsl-legalize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the `sv_coverage` / `sv_innercoverage` / `sv_depthgreaterequal`
// branches of `getGLSLSystemValueInfo` (slang-ir-glsl-legalize.cpp:543-600).
// These are the three fragment system values whose GLSL form is not a
// drop-in rename: `SV_Coverage` is a 32-bit uint in HLSL but an *array* of
// int in GLSL, so the legalizer sets both a `requiredType` of int and an
// `arrayIndex` of 0 and picks a different builtin per direction
// (`gl_SampleMaskIn` when read, `gl_SampleMask` when written);
// `SV_InnerCoverage` is a uint in HLSL but a bool in GLSL behind an NV
// extension; and `SV_DepthGreaterEqual` is not a builtin at all but a layout
// qualifier on `gl_FragDepth`. CHECK asserts each of those forms, including
// the `[0]` subscripts and the int/uint conversions the type mismatch forces,
// so a regression that dropped the array indexing or the conversion would be
// visible.
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage fragment

struct FIn
{
    float4 pos : SV_Position;
    uint cov : SV_Coverage;
    uint inner : SV_InnerCoverage;
}
struct FOut
{
    float4 color : SV_Target;
    uint cov : SV_Coverage;
    float depth : SV_DepthGreaterEqual;
}

[shader("fragment")]
FOut main(FIn i)
{
    FOut o;
    o.color = i.pos + float(i.cov + i.inner);
    o.cov = i.cov;
    o.depth = 0.25;
    return o;
}

//GLSL-DAG: #extension GL_NV_conservative_raster_underestimation : require
//GLSL-DAG: #extension GL_ARB_conservative_depth : require
// SV_DepthGreaterEqual is a layout qualifier on the depth builtin.
//GLSL: layout(depth_greater) out float gl_FragDepth;
// Reading SV_Coverage subscripts gl_SampleMaskIn and converts int -> uint;
// SV_InnerCoverage reads the bool NV builtin.
//GLSL: uint(gl_SampleMaskIn[0]) + uint(gl_FragFullyCoveredNV)
// Writing SV_Coverage subscripts gl_SampleMask and converts uint -> int.
//GLSL: gl_SampleMask[0] = int(
//GLSL: gl_FragDepth =
