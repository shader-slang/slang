// Check that extracting the sampler from one never-written combined texture-sampler both as a
// `SamplerState` and as a `SamplerComparisonState` yields a value of each type, rather than one
// value of the first type passed where the other is expected.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute -O0

// CHECK-DAG: SamplerState [[SAMPLER:[A-Za-z0-9_]+]];
// CHECK-DAG: SamplerComparisonState [[COMPARISON:[A-Za-z0-9_]+]];
// CHECK-DAG: .SampleLevel([[SAMPLER]],
// CHECK-DAG: .SampleCmpLevelZero([[COMPARISON]],

Texture2D<float> tex;
RWStructuredBuffer<float> outputBuffer;

[noinline]
float sampleWith(SamplerState s)
{
    return tex.SampleLevel(s, float2(0), 0);
}

[noinline]
float compareWith(SamplerComparisonState s)
{
    return tex.SampleCmpLevelZero(s, float2(0), 0);
}

[numthreads(1, 1, 1)]
void main()
{
    Sampler2D<float> combined;
    outputBuffer[0] = sampleWith(combined.__getSampler());
    outputBuffer[1] = compareWith(combined.__getComparisonSampler());
}
