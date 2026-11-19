//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute -Wno-using-uninitialized-variable

// A mixed ordinary/resource struct forces the uninitialized element to
// decompose during type legalization into an ordinary part and a special
// (resource) part. The opaque-handle diagnostic must remain located at the use
// that flows through the decomposition, so diagnostics for decomposed values
// keep pointing at user code.

uniform Texture2D gTex;
uniform SamplerState gSampler;
RWStructuredBuffer<float4> outputBuffer;

struct Mixed { float f; Texture2D t; };

cbuffer Settings
{
    uint index;
};

[numthreads(1, 1, 1)]
void main()
{
    Mixed defined = {1.0, gTex};
    Mixed m;
    Mixed arr[2] = {defined, m};
//CHECK:           ^ use of uninitialized opaque handle
//CHECK:           ^ use of uninitialized opaque handle 'Texture2D'.
    outputBuffer[0] = arr[index].t.SampleLevel(gSampler, float2(0), 0);
}
