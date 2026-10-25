// WGSL's comparison sampling builtins take only texture_depth_* textures, so a comparison sample of
// a Texture2D<float> is reported at compile time instead of emitting a textureSampleCompare call on
// a texture_2d<f32>, which the WGSL validator rejects at pipeline creation.
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CMP):-target wgsl -stage compute -entry sampleCmpMain
//DIAGNOSTIC_TEST:SIMPLE(filecheck=GATHER):-target wgsl -stage compute -entry gatherCmpMain
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
    // CMP: WGSL comparison sampling (SampleCmp, GatherCmp) needs a depth texture
    // CMP: colorTex.SampleCmpLevelZero
    outputBuffer[0].x = colorTex.SampleCmpLevelZero(cmpSampler, float2(0.5, 0.5), 0.5);
}

[numthreads(1, 1, 1)]
void gatherCmpMain()
{
    // GATHER: error[E41400]: static assertion failed
    // GATHER: WGSL comparison sampling (SampleCmp, GatherCmp) needs a depth texture
    // GATHER: colorTex.GatherCmp
    outputBuffer[0] = colorTex.GatherCmp(cmpSampler, float2(0.5, 0.5), 0.5);
}

// DEPTH: var depthTex_{{[0-9]+}} : texture_depth_2d;
// DEPTH: textureSampleCompareLevel(
[numthreads(1, 1, 1)]
void depthMain()
{
    outputBuffer[0].x = depthTex.SampleCmpLevelZero(cmpSampler, float2(0.5, 0.5), 0.5);
}
