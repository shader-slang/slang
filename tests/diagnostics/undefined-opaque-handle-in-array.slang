//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -entry main -stage compute

// Check that a never-written texture stored in an array beside a defined texture is reported as
// E56003, since SPIR-V for Vulkan has no `OpUndef` of an image (VUID-StandaloneSpirv-OpUndef-12464)
// and the texture cannot be removed without losing the other element of the array.

Texture2D<float4> gTexture;
SamplerState gSampler;
RWStructuredBuffer<float4> outputBuffer;
cbuffer Settings
{
    uint index;
};

[numthreads(1, 1, 1)]
void main()
{
    Texture2D<float4> t;
    Texture2D<float4> textures[2] = { gTexture, t };
//CHECK:                            ^ use of uninitialized variable
//CHECK:                            ^ use of uninitialized variable 't'
//CHECK:                            ^ use of uninitialized opaque handle
//CHECK:                            ^ use of uninitialized opaque handle 'Texture2D'.
    outputBuffer[0] = textures[index].SampleLevel(gSampler, float2(0), 0);
}
