// Cube and cube-array textures have no texel-offset sampling overloads in the HLSL API, and
// `OpImage*Dref*` comparison sampling is invalid on 3D images, so those calls are reported at
// compile time instead of silently emitting `ConstOffset` on a cube image or an invalid
// `OpImage*Dref*` on a 3D image. The WGSL offset helper restricts offsets to 2D and 3D shapes.
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CUBE_SAMPLE,exit-code=2):-target spirv -stage fragment -entry cubeSampleOffset
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CUBE_LEVEL,exit-code=2):-target spirv -stage fragment -entry cubeSampleLevelOffset
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CUBE_ARRAY_GATHER,exit-code=2):-target spirv -stage fragment -entry cubeArrayGatherOffset
//DIAGNOSTIC_TEST:SIMPLE(filecheck=COMBINED_CUBE_SAMPLE,exit-code=2):-target spirv -stage fragment -entry combinedCubeSampleOffset
//DIAGNOSTIC_TEST:SIMPLE(filecheck=D3_CMP,exit-code=2):-target spirv -stage fragment -entry tex3DSampleCmp
//DIAGNOSTIC_TEST:SIMPLE(filecheck=D3_CMP_LEVEL,exit-code=2):-target spirv -stage fragment -entry tex3DSampleCmpLevel
//DIAGNOSTIC_TEST:SIMPLE(filecheck=DEPTH3D_CMP,exit-code=2):-target spirv -stage fragment -entry depth3DSampleCmpLevelZero
//DIAGNOSTIC_TEST:SIMPLE(filecheck=WGSL_CUBE,exit-code=2):-target wgsl -stage fragment -entry wgslCubeSampleOffset
//DIAGNOSTIC_TEST:SIMPLE(filecheck=WGSL_1D,exit-code=2):-target wgsl -stage fragment -entry wgsl1DSampleOffset
// Offset-free cube forms are unaffected by the shape checks.
//TEST:SIMPLE(filecheck=OK):-target spirv-asm -stage fragment -entry cubeFormsStillWork

TextureCube<float4> cubeTex;
TextureCubeArray<float4> cubeArrayTex;
Texture3D<float4> tex3D;
DepthTexture3D depthTex3D;
Texture1D<float4> tex1D;
SamplerState samplerState;
SamplerComparisonState comparisonSampler;

SamplerCube combinedCubeSampler;

// CUBE_SAMPLE: error[E41400]: static assertion failed
// CUBE_SAMPLE: Sample with offset is not supported for TextureCube or TextureCubeArray
float4 cubeSampleOffset(float4 p: SV_Position) : SV_Target
{
    return cubeTex.Sample(samplerState, normalize(p.xyz), int2(1, 0));
}

// CUBE_LEVEL: error[E41400]: static assertion failed
// CUBE_LEVEL: SampleLevel with offset is not supported for TextureCube or TextureCubeArray
float4 cubeSampleLevelOffset(float4 p: SV_Position) : SV_Target
{
    return cubeTex.SampleLevel(samplerState, normalize(p.xyz), 0.0, int2(1, 0));
}

// CUBE_ARRAY_GATHER: error[E41400]: static assertion failed
// CUBE_ARRAY_GATHER: Gather with offset is not supported for TextureCube or TextureCubeArray
float4 cubeArrayGatherOffset(float4 p: SV_Position) : SV_Target
{
    return cubeArrayTex.GatherRed(samplerState, float4(normalize(p.xyz), 0), int2(1, 0));
}

// COMBINED_CUBE_SAMPLE: error[E41400]: static assertion failed
// COMBINED_CUBE_SAMPLE: Sample with offset is not supported for TextureCube or TextureCubeArray
float4 combinedCubeSampleOffset(float4 p: SV_Position) : SV_Target
{
    return combinedCubeSampler.Sample(normalize(p.xyz), int2(1, 0));
}

// D3_CMP: error[E41400]: static assertion failed
// D3_CMP: Comparison sampling is not supported for 3D textures
float4 tex3DSampleCmp(float4 p: SV_Position) : SV_Target
{
    return float4(tex3D.SampleCmp(comparisonSampler, p.xyz, 0.5), 0, 0, 1);
}

// D3_CMP_LEVEL: error[E41400]: static assertion failed
// D3_CMP_LEVEL: Comparison sampling is not supported for 3D textures
float4 tex3DSampleCmpLevel(float4 p: SV_Position) : SV_Target
{
    return float4(tex3D.SampleCmpLevel(comparisonSampler, p.xyz, 0.5, 0.0), 0, 0, 1);
}

// DEPTH3D_CMP: error[E41400]: static assertion failed
// DEPTH3D_CMP: Comparison sampling is not supported for 3D textures
float4 depth3DSampleCmpLevelZero(float4 p: SV_Position) : SV_Target
{
    return float4(depthTex3D.SampleCmpLevelZero(comparisonSampler, p.xyz, 0.5), 0, 0, 1);
}

// WGSL_CUBE: error[E41400]: static assertion failed
// WGSL_CUBE: WGSL texture shape must be 2D or 3D when sampling with texel offsets
float4 wgslCubeSampleOffset(float4 p: SV_Position) : SV_Target
{
    return cubeTex.Sample(samplerState, normalize(p.xyz), int2(1, 0));
}

// WGSL_1D: error[E41400]: static assertion failed
// WGSL_1D: WGSL texture shape must be 2D or 3D when sampling with texel offsets
float4 wgsl1DSampleOffset(float4 p: SV_Position) : SV_Target
{
    return tex1D.Sample(samplerState, p.x, 1);
}

// OK: OpImageSampleExplicitLod
// OK: Grad
// OK: OpImageGather
float4 cubeFormsStillWork(float4 p: SV_Position) : SV_Target
{
    float4 result = cubeTex.SampleGrad(samplerState, normalize(p.xyz), ddx(p.xyz), ddy(p.xyz));
    result += cubeTex.SampleLevel(samplerState, normalize(p.xyz), 0.0);
    result += cubeTex.GatherRed(samplerState, normalize(p.xyz));
    return result;
}
