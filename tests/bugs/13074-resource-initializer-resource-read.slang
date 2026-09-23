// The result of a resource-content load depends on mutable state outside the initializer's SSA
// operands. Moving this initializer could therefore change which texture it selects.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute -validate-ir

Texture2D<float4> textures[2];
RWStructuredBuffer<uint> textureIndices;

static Texture2D<float4> cachedTexture = textures[textureIndices[0]];
//CHECK:                 ^^^^^^^^^^^^^ unsupported file-scope static resource initializer
//CHECK:                 ^^^^^^^^^^^^^ initializer for file-scope static resource 'cachedTexture' has a side effect or reads mutable state; initialize the resource explicitly in an entry point instead

RWStructuredBuffer<float4> output;

[numthreads(1, 1, 1)]
void main()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0));
}
