//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Characterizes that SV_Barycentrics selects gl_BaryCoordEXT or gl_BaryCoordNoPerspEXT depending on whether the field carries the noperspective modifier, alongside the fragment mappings for SV_RenderTargetArrayIndex, SV_ViewportArrayIndex, SV_SampleIndex, SV_VulkanSamplePosition and SV_ShadingRate.
//META: intent=characterization
//META: covers=source/slang/slang-ir-glsl-legalize.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins the `sv_barycentrics` branch of `getGLSLSystemValueInfo`
// (slang-ir-glsl-legalize.cpp:925-949) together with the neighbouring
// fragment system values. The barycentrics branch is the interesting one: it
// is the only mapping in this table that inspects an *interpolation modifier*
// rather than just the semantic name, walking the chain of enclosing
// parameters looking for `IRInterpolationMode::NoPerspective` and switching
// the builtin name when it finds one. The two entry points below differ only
// in that modifier, so CHECK can assert the fork directly: `perspMain` gets
// `gl_BaryCoordEXT` and `noPerspMain` gets `gl_BaryCoordNoPerspEXT`. The
// perspective entry point also carries the neighbouring semantics so the
// int-typed gl_Layer / gl_ViewportIndex / gl_SampleID conversions and the
// input-side `gl_ShadingRateEXT` spelling are pinned in the same run.
//TEST:SIMPLE(filecheck=PERSP):-target glsl -entry perspMain -stage fragment
//TEST:SIMPLE(filecheck=NOPERSP):-target glsl -entry noPerspMain -stage fragment

struct FInPersp
{
    float4 pos : SV_Position;
    float3 bary : SV_Barycentrics;
    uint layer : SV_RenderTargetArrayIndex;
    uint vp : SV_ViewportArrayIndex;
    uint sampleIdx : SV_SampleIndex;
    float2 samplePos : SV_VulkanSamplePosition;
    uint rate : SV_ShadingRate;
}
struct FInNoPersp
{
    float4 pos : SV_Position;
    noperspective float3 bary : SV_Barycentrics;
}

[shader("fragment")]
float4 perspMain(FInPersp i) : SV_Target
{
    return float4(i.bary, 1.0) + float(i.sampleIdx) + float4(i.samplePos, 0, 0) +
           float(i.layer + i.vp + i.rate);
}

[shader("fragment")]
float4 noPerspMain(FInNoPersp i) : SV_Target
{
    return float4(i.bary, 1.0);
}

//PERSP-DAG: #extension GL_EXT_fragment_shader_barycentric : require
//PERSP-DAG: #extension GL_EXT_fragment_shading_rate : require
// Without noperspective, the perspective-correct barycentric builtin is used.
// gl_SampleID / gl_Layer / gl_ViewportIndex are int in GLSL, hence the casts;
// on the input side SV_ShadingRate is gl_ShadingRateEXT and stays uint.
//PERSP: vec4(gl_BaryCoordEXT, 1.0) + float(uint(gl_SampleID)) + vec4(gl_SamplePosition, 0.0, 0.0) + float(uint(gl_Layer) + uint(gl_ViewportIndex) + gl_ShadingRateEXT)

//NOPERSP: #extension GL_EXT_fragment_shader_barycentric : require
// The same semantic with `noperspective` selects the no-perspective builtin.
//NOPERSP: vec4(gl_BaryCoordNoPerspEXT, 1.0)
//NOPERSP-NOT: gl_BaryCoordEXT
