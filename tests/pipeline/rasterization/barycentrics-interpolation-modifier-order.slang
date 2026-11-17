// barycentrics-interpolation-modifier-order.slang

// An `SV_Barycentrics` input declared `noperspective` maps to the non-perspective barycentric
// builtin. A sampling-location modifier (`centroid` or `sample`) can be written before or after
// `noperspective`, and both orders must select the same builtin, for an entry-point parameter
// and for a struct field. Direct SPIR-V chooses the builtin in `maybeEmitSystemVal`; GLSL and
// SPIR-V via GLSL choose it in `getGLSLSystemValueInfo`. The two sites see the decorations in
// opposite orders, so every order runs on every path. The `linear` cases check that `linear`
// plus a sampling-location modifier still maps to the perspective builtin.

//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry paramMain -stage fragment -DFIRST=centroid -DSECOND=noperspective
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry paramMain -stage fragment -DFIRST=noperspective -DSECOND=centroid
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry paramMain -stage fragment -DFIRST=sample -DSECOND=noperspective
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry paramMain -stage fragment -DFIRST=noperspective -DSECOND=sample
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry fieldMain -stage fragment -DFIRST=centroid -DSECOND=noperspective
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry fieldMain -stage fragment -DFIRST=noperspective -DSECOND=centroid
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry fieldMain -stage fragment -DFIRST=sample -DSECOND=noperspective
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry fieldMain -stage fragment -DFIRST=noperspective -DSECOND=sample

//TEST:SIMPLE(filecheck=SPIRV):-target spirv -emit-spirv-via-glsl -entry paramMain -stage fragment -DFIRST=centroid -DSECOND=noperspective
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -emit-spirv-via-glsl -entry paramMain -stage fragment -DFIRST=noperspective -DSECOND=centroid
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -emit-spirv-via-glsl -entry paramMain -stage fragment -DFIRST=sample -DSECOND=noperspective
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -emit-spirv-via-glsl -entry paramMain -stage fragment -DFIRST=noperspective -DSECOND=sample
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -emit-spirv-via-glsl -entry fieldMain -stage fragment -DFIRST=centroid -DSECOND=noperspective
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -emit-spirv-via-glsl -entry fieldMain -stage fragment -DFIRST=noperspective -DSECOND=centroid
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -emit-spirv-via-glsl -entry fieldMain -stage fragment -DFIRST=sample -DSECOND=noperspective
//TEST:SIMPLE(filecheck=SPIRV):-target spirv -emit-spirv-via-glsl -entry fieldMain -stage fragment -DFIRST=noperspective -DSECOND=sample

//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry paramMain -stage fragment -DFIRST=centroid -DSECOND=noperspective
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry paramMain -stage fragment -DFIRST=noperspective -DSECOND=centroid
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry paramMain -stage fragment -DFIRST=sample -DSECOND=noperspective
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry paramMain -stage fragment -DFIRST=noperspective -DSECOND=sample
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry fieldMain -stage fragment -DFIRST=centroid -DSECOND=noperspective
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry fieldMain -stage fragment -DFIRST=noperspective -DSECOND=centroid
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry fieldMain -stage fragment -DFIRST=sample -DSECOND=noperspective
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry fieldMain -stage fragment -DFIRST=noperspective -DSECOND=sample

//TEST:SIMPLE(filecheck=SPIRV-PERSP):-target spirv -entry paramMain -stage fragment -DFIRST=linear -DSECOND=centroid
//TEST:SIMPLE(filecheck=SPIRV-PERSP):-target spirv -entry paramMain -stage fragment -DFIRST=sample -DSECOND=linear
//TEST:SIMPLE(filecheck=SPIRV-PERSP):-target spirv -emit-spirv-via-glsl -entry paramMain -stage fragment -DFIRST=linear -DSECOND=centroid
//TEST:SIMPLE(filecheck=SPIRV-PERSP):-target spirv -emit-spirv-via-glsl -entry paramMain -stage fragment -DFIRST=sample -DSECOND=linear
//TEST:SIMPLE(filecheck=GLSL-PERSP):-target glsl -entry paramMain -stage fragment -DFIRST=linear -DSECOND=centroid
//TEST:SIMPLE(filecheck=GLSL-PERSP):-target glsl -entry paramMain -stage fragment -DFIRST=sample -DSECOND=linear
//TEST:SIMPLE(filecheck=SPIRV-PERSP):-target spirv -entry fieldMain -stage fragment -DFIRST=centroid -DSECOND=linear
//TEST:SIMPLE(filecheck=SPIRV-PERSP):-target spirv -emit-spirv-via-glsl -entry fieldMain -stage fragment -DFIRST=linear -DSECOND=sample
//TEST:SIMPLE(filecheck=GLSL-PERSP):-target glsl -entry fieldMain -stage fragment -DFIRST=linear -DSECOND=sample

// SPIRV-NOT: BuiltIn BaryCoordKHR
// SPIRV: OpDecorate %{{.*}} BuiltIn BaryCoordNoPerspKHR
// SPIRV-NOT: BuiltIn BaryCoordKHR

// GLSL-NOT: gl_BaryCoordEXT
// GLSL: gl_BaryCoordNoPerspEXT
// GLSL-NOT: gl_BaryCoordEXT

// SPIRV-PERSP-NOT: BuiltIn BaryCoordNoPerspKHR
// SPIRV-PERSP: OpDecorate %{{.*}} BuiltIn BaryCoordKHR
// SPIRV-PERSP-NOT: BuiltIn BaryCoordNoPerspKHR

// GLSL-PERSP-NOT: gl_BaryCoordNoPerspEXT
// GLSL-PERSP: gl_BaryCoordEXT
// GLSL-PERSP-NOT: gl_BaryCoordNoPerspEXT

struct FragmentInput
{
    FIRST SECOND float3 bary : SV_Barycentrics;
};

[shader("fragment")]
float4 paramMain(FIRST SECOND float3 bary : SV_Barycentrics) : SV_Target
{
    return float4(bary, 1.0);
}

[shader("fragment")]
float4 fieldMain(FragmentInput input) : SV_Target
{
    return float4(input.bary, 1.0);
}
