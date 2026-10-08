// An immutable pointer prevents writes through the pointer, but it can still address resource-backed
// storage. This helper returns an address into a structured buffer, so loading through the returned
// pointer reads resource contents.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target spirv -entry main -stage compute -validate-ir

Texture2D<float4> textures[2];
RWStructuredBuffer<uint> textureIndices;

[__readNone]
[noinline]
ImmutablePtr<uint> getTextureIndexAddress()
{
    return ImmutablePtr<uint>(&textureIndices[0]);
}

static Texture2D<float4> cachedTexture = textures[*getTextureIndexAddress()];
//CHECK:                 ^^^^^^^^^^^^^ cannot prove that a mutable resource initializer can be moved safely
//CHECK:                 ^^^^^^^^^^^^^ Slang cannot prove that moving the initializer for the mutable resource variable 'cachedTexture' to each entry point preserves observable behavior. The initializer may fail to return normally, may have externally observable side effects, or may read preexisting mutable storage, resource contents, or non-resource data from an explicit parameter group; initialize the resource explicitly in an entry point instead

RWStructuredBuffer<float4> output;

[numthreads(1, 1, 1)]
void main()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0));
}
