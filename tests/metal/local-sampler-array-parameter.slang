// In this test, `sampleTexture` accepts a fixed-size sampler array by value. The
// `transformParamsToConstRef` pass copies `samplers` into a local array before the call. The `MTL`
// check verifies the emitted `thread array<sampler, ...>` local. The `LIB` test verifies that
// Apple's Metal compiler accepts the generated source.

//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain -O0
//TEST:SIMPLE(filecheck=MTL): -target metal -stage compute -entry computeMain -O0 -minimum-slang-optimization
//TEST:SIMPLE(filecheck=LIB): -target metallib -stage compute -entry computeMain -O0

uniform Texture2D<float4> texture;
uniform SamplerState samplers[2];
RWStructuredBuffer<float4> output;

[noinline]
float4 sampleTexture(SamplerState values[2])
{
    return texture.SampleLevel(values[0], float2(0, 0), 0);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadID : SV_DispatchThreadID)
{
    output[threadID.x] = sampleTexture(samplers);
}

// MTL: thread array<sampler,
// LIB: define void @computeMain
