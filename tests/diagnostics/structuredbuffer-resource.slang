//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Direct resource types as StructuredBuffer elements

StructuredBuffer<Texture2D<float>> textureBuffer;
//CHECK:                           ^^^^^^^^^^^^^ resource type in StructuredBuffer
//CHECK:                           ^^^^^^^^^^^^^ StructuredBuffer element type 'Texture2D' must not be a resource type or contain an opaque handle type

StructuredBuffer<SamplerState> samplerBuffer;
//CHECK:                       ^^^^^^^^^^^^^ resource type in StructuredBuffer
//CHECK:                       ^^^^^^^^^^^^^ StructuredBuffer element type 'SamplerState' must not be a resource type or contain an opaque handle type

StructuredBuffer<StructuredBuffer<float>> nestedBuffer;
//CHECK:                                  ^^^^^^^^^^^^ resource type in StructuredBuffer
//CHECK:                                  ^^^^^^^^^^^^ StructuredBuffer element type 'StructuredBuffer<float>' must not be a resource type or contain an opaque handle type

RWStructuredBuffer<Texture2D<float>> rwTextureBuffer;
//CHECK:                             ^^^^^^^^^^^^^^^ resource type in StructuredBuffer
//CHECK:                             ^^^^^^^^^^^^^^^ StructuredBuffer element type 'Texture2D' must not be a resource type or contain an opaque handle type

RWStructuredBuffer<float> output;

[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint i = dispatchThreadID.x;
    float result = textureBuffer[i].Sample(samplerBuffer[i], float2(0, 0)).x 
                 + nestedBuffer[i][0];

    rwTextureBuffer[i] = textureBuffer[i];
    output[i] = result;
}
