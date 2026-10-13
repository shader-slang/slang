// A `ReadNone` function can return the address of mutable global storage without reading that
// storage itself. The load through the returned pointer still makes this initializer depend on
// when it executes.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute -validate-ir

Texture2D<float4> textures[2];
static uint textureIndex = 0;

[__readNone]
[noinline]
Ptr<uint> getTextureIndexAddress()
{
    return &textureIndex;
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
