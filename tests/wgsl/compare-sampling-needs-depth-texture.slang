// WGSL's comparison sampling builtins take only texture_depth_* textures, so a comparison sample of
// a Texture2D<float> is reported at compile time instead of emitting a textureSampleCompare call on
// a texture_2d<f32>, which the WGSL validator rejects at pipeline creation.
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CMP):-target wgsl -stage compute -entry sampleCmpMain
//DIAGNOSTIC_TEST:SIMPLE(filecheck=GATHER):-target wgsl -stage compute -entry gatherCmpMain
//DIAGNOSTIC_TEST:SIMPLE(filecheck=GATHEROFF):-target wgsl -stage compute -entry gatherCmpOffsetMain
//DIAGNOSTIC_TEST:SIMPLE(filecheck=LZOFF):-target wgsl -stage compute -entry sampleCmpLevelZeroOffsetMain
//DIAGNOSTIC_TEST:SIMPLE(filecheck=SCMP):-target wgsl -stage fragment -entry sampleCmpFragment
//DIAGNOSTIC_TEST:SIMPLE(filecheck=SCMPOFF):-target wgsl -stage fragment -entry sampleCmpOffsetFragment
// A depth texture still compiles.
//TEST:SIMPLE(filecheck=DEPTH):-target wgsl -stage compute -entry depthMain

Texture2D<float> colorTex;
DepthTexture2D depthTex;
SamplerComparisonState cmpSampler;
RWStructuredBuffer<float4> outputBuffer;

[numthreads(1, 1, 1)]
void sampleCmpMain()
{
    // CMP: error[E41400]: static assertion failed
    // CMP: WGSL comparison sampling (SampleCmp, GatherCmp) needs a depth texture: declare the texture as DepthTexture2D, DepthTexture2DArray, DepthTextureCube or DepthTextureCubeArray
    // CMP: colorTex.SampleCmpLevelZero
    outputBuffer[0].x = colorTex.SampleCmpLevelZero(cmpSampler, float2(0.5, 0.5), 0.5);
}

[numthreads(1, 1, 1)]
void gatherCmpMain()
{
    // GATHER: error[E41400]: static assertion failed
    // GATHER: WGSL comparison sampling (SampleCmp, GatherCmp) needs a depth texture: declare the texture as DepthTexture2D, DepthTexture2DArray, DepthTextureCube or DepthTextureCubeArray
    // GATHER: colorTex.GatherCmp
    outputBuffer[0] = colorTex.GatherCmp(cmpSampler, float2(0.5, 0.5), 0.5);
}

[numthreads(1, 1, 1)]
void gatherCmpOffsetMain()
{
    // GATHEROFF: error[E41400]: static assertion failed
    // GATHEROFF: WGSL comparison sampling (SampleCmp, GatherCmp) needs a depth texture: declare the texture as DepthTexture2D, DepthTexture2DArray, DepthTextureCube or DepthTextureCubeArray
    // GATHEROFF: colorTex.GatherCmp
    outputBuffer[0] = colorTex.GatherCmp(cmpSampler, float2(0.5, 0.5), 0.5, int2(1, 0));
}

[numthreads(1, 1, 1)]
void sampleCmpLevelZeroOffsetMain()
{
    // LZOFF: error[E41400]: static assertion failed
    // LZOFF: WGSL comparison sampling (SampleCmp, GatherCmp) needs a depth texture: declare the texture as DepthTexture2D, DepthTexture2DArray, DepthTextureCube or DepthTextureCubeArray
    // LZOFF: colorTex.SampleCmpLevelZero
    outputBuffer[0].x = colorTex.SampleCmpLevelZero(cmpSampler, float2(0.5, 0.5), 0.5, int2(1, 0));
}

float4 sampleCmpFragment(float4 p: SV_Position) : SV_Target
{
    // SCMP: error[E41400]: static assertion failed
    // SCMP: WGSL comparison sampling (SampleCmp, GatherCmp) needs a depth texture: declare the texture as DepthTexture2D, DepthTexture2DArray, DepthTextureCube or DepthTextureCubeArray
    // SCMP: colorTex.SampleCmp
    return colorTex.SampleCmp(cmpSampler, p.xy, 0.5);
}

float4 sampleCmpOffsetFragment(float4 p: SV_Position) : SV_Target
{
    // SCMPOFF: error[E41400]: static assertion failed
    // SCMPOFF: WGSL comparison sampling (SampleCmp, GatherCmp) needs a depth texture: declare the texture as DepthTexture2D, DepthTexture2DArray, DepthTextureCube or DepthTextureCubeArray
    // SCMPOFF: colorTex.SampleCmp
    return colorTex.SampleCmp(cmpSampler, p.xy, 0.5, int2(1, 0));
}

// DEPTH: var depthTex_{{[0-9]+}} : texture_depth_2d;
// DEPTH: textureSampleCompareLevel(
[numthreads(1, 1, 1)]
void depthMain()
{
    outputBuffer[0].x = depthTex.SampleCmpLevelZero(cmpSampler, float2(0.5, 0.5), 0.5);
}
