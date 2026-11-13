//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute

// A never-written texture passed to a `[noinline]` function cannot be removed by inlining. SPIR-V
// for Vulkan has no undefined image (VUID-StandaloneSpirv-OpUndef-12464), so the compiler reports
// the use instead of emitting an invalid module. See
// https://github.com/shader-slang/slang/issues/14076.

Texture2D<float4> gTexture;
SamplerState gSampler;
RWStructuredBuffer<float4> outputBuffer;
cbuffer Settings
{
    bool useUndefined;
};

[noinline]
float4 readTexture(Texture2D<float4> t)
{
    return t.SampleLevel(gSampler, float2(0), 0);
}

[numthreads(1, 1, 1)]
void main()
{
    Texture2D<float4> t;
    float4 r = readTexture(gTexture);
    if (useUndefined)
        r = readTexture(t);
//CHECK:               ^ use of uninitialized variable
//CHECK:               ^ use of uninitialized variable 't'
//CHECK:               ^ use of uninitialized opaque handle
//CHECK:               ^ use of uninitialized opaque handle 'Texture2D'.
    outputBuffer[0] = r;
}
