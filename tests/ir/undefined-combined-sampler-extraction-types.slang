// Check that extracting the sampler from one never-written combined texture-sampler both as a
// `SamplerState` and as a `SamplerComparisonState` yields a value of each type, rather than one
// value of the first type passed where the other is expected. Direct SPIR-V expands the combined
// value into a texture and a `SamplerState`, so the comparison sampler is the case that needs a
// value of its own type. Each call is then specialized to an undefined sampler of its parameter's
// type, and the sample through it is removed.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -validate-ir
//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry main -stage compute -validate-ir -O0

// CHECK: %main = OpFunction
// CHECK: OpFunctionCall %float %sampleWith{{$}}
// CHECK: OpFunctionCall %float %compareWith{{$}}
// CHECK: OpFunctionEnd

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
