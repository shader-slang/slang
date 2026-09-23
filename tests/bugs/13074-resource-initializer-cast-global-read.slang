// An address cast must not hide a read from mutable global storage. The initializer still depends
// on when `textureIndex` is read, so moving it into the entry point is unsupported.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute -validate-ir

Texture2D<float4> textures[2];
static uint textureIndex = 0;

static Texture2D<float4> cachedTexture = textures[*reinterpret<Ptr<uint>>(&textureIndex)];
//CHECK:                 ^^^^^^^^^^^^^ unsupported file-scope static resource initializer
//CHECK:                 ^^^^^^^^^^^^^ initializer for file-scope static resource 'cachedTexture' has a side effect or reads mutable state; initialize the resource explicitly in an entry point instead

RWStructuredBuffer<float4> output;

[numthreads(1, 1, 1)]
void main()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0));
}
