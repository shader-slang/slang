// A function with no side effects may still read mutable global state. Moving this resource
// initializer could therefore change which value `chooseTexture` reads.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage compute -validate-ir

Texture2D<float4> textures[2];
static uint textureIndex = 0;

[noinline]
Texture2D<float4> chooseTexture()
{
    return textures[textureIndex];
}

static Texture2D<float4> cachedTexture = chooseTexture();
//CHECK:                 ^^^^^^^^^^^^^ unsupported file-scope static resource initializer
//CHECK:                 ^^^^^^^^^^^^^ initializer for file-scope static resource 'cachedTexture' has a side effect or reads mutable state; initialize the resource explicitly in an entry point instead

RWStructuredBuffer<float4> output;

[numthreads(1, 1, 1)]
void main()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0));
}
