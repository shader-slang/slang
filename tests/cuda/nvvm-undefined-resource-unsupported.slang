// Choosing an unused CUDA sampler placeholder must not admit undefined texture handles or
// aggregates containing resources. These checks compile only and never dispatch a resource read.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry textureCase -stage compute -capability cuda_sm_8_0 -O0 -o -
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry textureCase -stage compute -capability cuda_sm_8_0 -O3 -o -
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry aggregateCase -stage compute -capability cuda_sm_8_0 -O0 -o -
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry aggregateCase -stage compute -capability cuda_sm_8_0 -O3 -o -
#pragma warning(disable: 41016)

RWStructuredBuffer<float4> outputBuffer;
SamplerState boundSampler;

struct Resources
{
    Texture2D<float4> texture;
    SamplerState sampler;
};

[noinline]
float4 sampleResource(Resources resources)
{
    return resources.texture.SampleLevel(resources.sampler, float2(0.5), 0);
}

[numthreads(1, 1, 1)]
void textureCase()
{
    Texture2D<float4> texture;
    outputBuffer[0] = texture.SampleLevel(boundSampler, float2(0.5), 0);
}

[numthreads(1, 1, 1)]
void aggregateCase()
{
    Resources resources;
    outputBuffer[0] = sampleResource(resources);
}
//CHECK: direct NVVM lowering does not support Slang IR instruction or shape 'LoadFromUninitializedMemory'
