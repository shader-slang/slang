//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

struct HasResource
{
    Texture2D tex;
}

StructuredBuffer<HasResource> bufferArray[4];
//CHECK:                      ^^^^^^^^^^^ resource type in StructuredBuffer
//CHECK:                      ^^^^^^^^^^^ StructuredBuffer element type 'HasResource' cannot contain resource or opaque handle types

RWStructuredBuffer<Texture2D> rwBufferArray[2];
//CHECK:                      ^^^^^^^^^^^^^ resource type in StructuredBuffer
//CHECK:                      ^^^^^^^^^^^^^ StructuredBuffer element type 'Texture2D' cannot contain resource or opaque handle types

SamplerState sampler;
RWStructuredBuffer<float> output;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint i = dispatchThreadID.x;

    // Force usage of all array elements and struct members
    float result = bufferArray[i % 4][i].tex.Sample(sampler, float2(0, 0)).x;
    rwBufferArray[i % 2][i] = bufferArray[0][i].tex;
    output[i] = result;
}

