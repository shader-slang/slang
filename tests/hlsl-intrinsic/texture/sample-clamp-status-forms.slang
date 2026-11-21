// The clamp and residency forms of the sampling methods, per the HLSL shape/method table:
// cube textures take (clamp) / (clamp, status) on the implicit-LOD methods and status-only forms
// on the explicit-LOD methods, and noncube textures take (offset, clamp) / (offset, clamp, status)
// on SampleBias and the SampleCmp family. The SPIR-V run checks the image operands (MinLod, the
// status store) as well as overload selection; the GLSL run covers the clamp forms that lower
// through GL_ARB_sparse_texture_clamp (GLSL has no residency forms, so those stay out of glslMain);
// the Metal run covers the hand-written MSL clamp bodies.
// The profile is sm_6_8 so the SampleCmpLevel (SM 6.7 on HLSL) and the comparison clamp forms
// clear their capability checks without profile-upgrade warnings.
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage fragment -entry fragmentMain -profile sm_6_8
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage fragment -entry fragmentMain
//TEST:SIMPLE(filecheck=GLSL):-target glsl -profile glsl_460 -stage fragment -entry glslMain
//TEST:SIMPLE(filecheck=MTL):-target metal -stage fragment -entry metalMain

TextureCube<float4> cubeTex;
Texture1D<float4> tex1D;
Texture2D<float4> tex2D;
Texture3D<float4> tex3D;
Texture2DArray<float4> tex2DArray;
SamplerState samplerState;

// Depth forms use the SampleCmp family.
DepthTextureCube depthCube;
DepthTexture1D depth1D;
DepthTexture2D depth2D;
SamplerComparisonState comparisonSampler;

// Combined samplers.
SamplerCube combinedCube;
SamplerCubeShadow combinedCubeShadow;
Sampler2D combined2D;

float4 fragmentMain(float4 p: SV_Position) : SV_Target
{
    float4 result = 0;

    // Cube clamp forms.
    // SPV: %{{[A-Za-z0-9_]+}} = OpImageSampleImplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} MinLod
    result += cubeTex.Sample(samplerState, normalize(p.xyz), 0.5);
    // HLSL: .Sample(samplerState_{{[0-9]*}}, normalize(
    // SPV: OpImageSampleImplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Bias|MinLod
    result += cubeTex.SampleBias(samplerState, normalize(p.xyz), 0.25, 0.5);
    // HLSL: .SampleBias(samplerState_{{[0-9]*}}, normalize(
    // SPV: OpImageSampleExplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Grad|MinLod
    result += cubeTex.SampleGrad(samplerState, normalize(p.xyz), ddx(p.xyz), ddy(p.xyz), 0.5);
    // HLSL: .SampleGrad(samplerState_{{[0-9]*}}, normalize(

    // Cube status forms.
    uint status;
    // SPV: %{{[A-Za-z0-9_]+}} = OpImageSparseSampleImplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} MinLod
    // SPV: OpStore %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}}
    result += cubeTex.Sample(samplerState, normalize(p.xyz), 0.5, status);
    // SPV: %{{[A-Za-z0-9_]+}} = OpImageSparseSampleImplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Bias|MinLod
    result += cubeTex.SampleBias(samplerState, normalize(p.xyz), 0.25, 0.5, status);
    // SPV: %{{[A-Za-z0-9_]+}} = OpImageSparseSampleExplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Grad|MinLod
    result += cubeTex.SampleGrad(samplerState, normalize(p.xyz), ddx(p.xyz), ddy(p.xyz), 0.5, status);
    // SPV: %{{[A-Za-z0-9_]+}} = OpImageSparseSampleExplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Lod %{{[A-Za-z0-9_.]+}}
    result += cubeTex.SampleLevel(samplerState, normalize(p.xyz), 0.0, status);

    // Cube comparison clamp and status forms.
    // SPV: %{{[A-Za-z0-9_]+}} = OpImageSampleDrefImplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} MinLod
    result += depthCube.SampleCmp(comparisonSampler, normalize(p.xyz), 0.5, 0.25);
    // SPV: OpImageSampleDrefImplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Bias|MinLod
    result += depthCube.SampleCmpBias(comparisonSampler, normalize(p.xyz), 0.5, 0.25, 0.5);
    // SPV: OpImageSampleDrefExplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Grad|MinLod
    result += depthCube.SampleCmpGrad(comparisonSampler, normalize(p.xyz), 0.5, ddx(p.xyz), ddy(p.xyz), 0.25);
    // SPV: %{{[A-Za-z0-9_]+}} = OpImageSparseSampleDrefExplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Lod
    result += depthCube.SampleCmpLevel(comparisonSampler, normalize(p.xyz), 0.5, 0.0, status);
    // SampleCmpLevelZero with status forwards to the LOD form with level 0 on SPIR-V.
    // SPV: %{{[A-Za-z0-9_]+}} = OpImageSparseSampleDrefExplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Lod %float_0
    result += depthCube.SampleCmpLevelZero(comparisonSampler, normalize(p.xyz), 0.5, status);
    // SPV: %{{[A-Za-z0-9_]+}} = OpImageSparseSampleDrefImplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} MinLod
    result += depthCube.SampleCmp(comparisonSampler, normalize(p.xyz), 0.5, 0.25, status);
    // SPV: %{{[A-Za-z0-9_]+}} = OpImageSparseSampleDrefImplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Bias|MinLod
    result += depthCube.SampleCmpBias(comparisonSampler, normalize(p.xyz), 0.5, 0.25, 0.5, status);
    // SPV: %{{[A-Za-z0-9_]+}} = OpImageSparseSampleDrefExplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Grad|MinLod
    result += depthCube.SampleCmpGrad(comparisonSampler, normalize(p.xyz), 0.5, ddx(p.xyz), ddy(p.xyz), 0.25, status);

    // Noncube (offset, clamp) and (offset, clamp, status) forms.
    // SPV: OpImageSampleImplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Bias|ConstOffset|MinLod
    result += tex2D.SampleBias(samplerState, p.xy, 0.25, int2(1, 0), 0.5);
    // SPV: OpImageSparseSampleImplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Bias|ConstOffset|MinLod
    result += tex2D.SampleBias(samplerState, p.xy, 0.25, int2(1, 0), 0.5, status);

    // The same (offset, clamp) form across the other noncube shapes: 1D, 3D and 2D array.
    // SPV: OpImageSampleImplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Bias|ConstOffset|MinLod
    result += tex1D.SampleBias(samplerState, p.x, 0.25, 1, 0.5);
    // SPV: OpImageSampleImplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Bias|ConstOffset|MinLod
    result += tex3D.SampleBias(samplerState, p.xyz, 0.25, int3(1, 0, 0), 0.5);
    // SPV: OpImageSampleImplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Bias|ConstOffset|MinLod
    result += tex2DArray.SampleBias(samplerState, float3(p.xy, 0), 0.25, int2(1, 0), 0.5);

    // The comparison (offset, clamp) forms on noncube shapes.
    // SPV: OpImageSampleDrefImplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} ConstOffset|MinLod
    result += float4(depth2D.SampleCmp(comparisonSampler, p.xy, 0.5, int2(1, 0), 0.25), 0, 0, 1);
    // SPV: OpImageSampleDrefImplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Bias|ConstOffset|MinLod
    result += float4(depth1D.SampleCmpBias(comparisonSampler, p.x, 0.5, 0.25, 1, 0.5), 0, 0, 1);
    // SPV: OpImageSampleDrefExplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Grad|ConstOffset|MinLod
    result += float4(depth2D.SampleCmpGrad(comparisonSampler, p.xy, 0.5, ddx(p.xy), ddy(p.xy), int2(1, 0), 0.25), 0, 0, 1);

    // Combined samplers exercise the twin wrappers.
    // SPV: %{{[A-Za-z0-9_]+}} = OpImageSampleImplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} MinLod
    result += combinedCube.Sample(normalize(p.xyz), 0.5);
    // SPV: OpImageSampleExplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Grad|MinLod
    result += combinedCube.SampleGrad(normalize(p.xyz), ddx(p.xyz), ddy(p.xyz), 0.5);
    // SPV: %{{[A-Za-z0-9_]+}} = OpImageSparseSampleImplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} MinLod
    result += combinedCube.Sample(normalize(p.xyz), 0.5, status);
    // SPV: OpImageSampleDrefImplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} MinLod
    result += combinedCubeShadow.SampleCmp(normalize(p.xyz), 0.5, 0.25);
    // SPV: %{{[A-Za-z0-9_]+}} = OpImageSparseSampleDrefImplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} MinLod
    result += combinedCubeShadow.SampleCmp(normalize(p.xyz), 0.5, 0.25, status);
    // SPV: OpImageSampleDrefExplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Grad|MinLod
    result += combinedCubeShadow.SampleCmpGrad(normalize(p.xyz), 0.5, ddx(p.xyz), ddy(p.xyz), 0.25);
    // SPV: %{{[A-Za-z0-9_]+}} = OpImageSparseSampleDrefExplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Grad|MinLod
    result += combinedCubeShadow.SampleCmpGrad(normalize(p.xyz), 0.5, ddx(p.xyz), ddy(p.xyz), 0.25, status);
    // SPV: OpImageSampleImplicitLod %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} Bias|ConstOffset|MinLod
    result += combined2D.SampleBias(p.xy, 0.25, int2(1, 0), 0.5);

    return result + status;
}

// GLSL: #extension GL_ARB_sparse_texture_clamp
// GLSL: textureClampARB(
// GLSL: textureGradClampARB(
// GLSL: textureOffsetClampARB(
float4 glslMain(float4 p: SV_Position) : SV_Target
{
    float4 result = 0;
    result += cubeTex.Sample(samplerState, normalize(p.xyz), 0.5);
    result += cubeTex.SampleBias(samplerState, normalize(p.xyz), 0.25, 0.5);
    result += cubeTex.SampleGrad(samplerState, normalize(p.xyz), ddx(p.xyz), ddy(p.xyz), 0.5);
    result += tex2D.SampleBias(samplerState, p.xy, 0.25, int2(1, 0), 0.5);
    result += combinedCube.Sample(normalize(p.xyz), 0.5);
    result += combinedCube.SampleGrad(normalize(p.xyz), ddx(p.xyz), ddy(p.xyz), 0.5);
    result += combined2D.SampleBias(p.xy, 0.25, int2(1, 0), 0.5);
    return result;
}

// MTL: min_lod_clamp(
// MSL documents the cube clamp form for bias and gradient sampling; the plain implicit-LOD
// clamp form has no MSL spelling, so metalMain exercises the bias and gradient bodies.
float4 metalMain(float4 p: SV_Position) : SV_Target
{
    float4 result = 0;
    result += cubeTex.SampleBias(samplerState, normalize(p.xyz), 0.25, 0.5);
    result += cubeTex.SampleGrad(samplerState, normalize(p.xyz), ddx(p.xyz), ddy(p.xyz), 0.5);
    result += tex2D.SampleBias(samplerState, p.xy, 0.25, int2(1, 0), 0.5);
    result += combinedCube.SampleBias(normalize(p.xyz), 0.25, 0.5);
    result += combinedCube.SampleGrad(normalize(p.xyz), ddx(p.xyz), ddy(p.xyz), 0.5);
    return result;
}
