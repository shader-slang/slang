// WGSL's sampling builtins take only f32 textures, so sampling an integer texture is reported at
// compile time instead of emitting textureSample* on a texture_2d<u32>, which the WGSL validator
// rejects at pipeline creation. Load and Gather of an integer texture stay valid.
//DIAGNOSTIC_TEST:SIMPLE(filecheck=SAMPLE):-target wgsl -stage compute -entry sampleLevelMain
//DIAGNOSTIC_TEST:SIMPLE(filecheck=GRAD):-target wgsl -stage compute -entry sampleGradMain
//TEST:SIMPLE(filecheck=OK): -target wgsl -stage compute -entry loadGatherMain

Texture2D<uint> ids;
SamplerState samp;
RWStructuredBuffer<uint4> outputBuffer;

[numthreads(1, 1, 1)]
void sampleLevelMain()
{
    // SAMPLE: error[E41400]: static assertion failed
    // SAMPLE: WGSL can only sample (Sample, SampleLevel, SampleGrad, SampleBias) a float texture
    // SAMPLE: ids.SampleLevel
    outputBuffer[0].x = ids.SampleLevel(samp, float2(0.5, 0.5), 0);
}

[numthreads(1, 1, 1)]
void sampleGradMain()
{
    // GRAD: error[E41400]: static assertion failed
    // GRAD: ids.SampleGrad
    outputBuffer[0].x = ids.SampleGrad(samp, float2(0.5, 0.5), float2(0.1, 0), float2(0, 0.1));
}

// OK: textureLoad(
// OK: textureGather(
[numthreads(1, 1, 1)]
void loadGatherMain()
{
    outputBuffer[0] = uint4(ids.Load(int3(1, 2, 0)), 0, 0, 0) + ids.GatherRed(samp, float2(0.5, 0.5));
}
