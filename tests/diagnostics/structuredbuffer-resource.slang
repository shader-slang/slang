//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv

// Direct resource types as StructuredBuffer elements

StructuredBuffer<Texture2D<float>> textureBuffer;
//CHECK:                           ^^^^^^^^^^^^^ resource type in StructuredBuffer
//CHECK:                           ^^^^^^^^^^^^^ StructuredBuffer element type 'Texture2D' cannot contain resource or opaque handle types

StructuredBuffer<SamplerState> samplerBuffer;
//CHECK:                       ^^^^^^^^^^^^^ resource type in StructuredBuffer
//CHECK:                       ^^^^^^^^^^^^^ StructuredBuffer element type 'SamplerState' cannot contain resource or opaque handle types

StructuredBuffer<StructuredBuffer<float>> nestedBuffer;
//CHECK:                                  ^^^^^^^^^^^^ resource type in StructuredBuffer
//CHECK:                                  ^^^^^^^^^^^^ StructuredBuffer element type 'StructuredBuffer<float>' cannot contain resource or opaque handle types

RWStructuredBuffer<Texture2D<float>> rwTextureBuffer;
//CHECK:                             ^^^^^^^^^^^^^^^ resource type in StructuredBuffer
//CHECK:                             ^^^^^^^^^^^^^^^ StructuredBuffer element type 'Texture2D' cannot contain resource or opaque handle types

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
