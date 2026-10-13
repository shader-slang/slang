// Loading a texture field, or a homogeneous fixed-size array of texture fields, from a parameter
// block copies resource identities. It does not read resource contents or non-resource parameter
// data, so moving either initializer to the start of each entry point preserves its behavior.

//TEST:SIMPLE: -target hlsl -entry main -stage compute -validate-ir -o 13074-resource-initializer-parameter-block-field.hlsl

struct Resources
{
    Texture2D<float4> texture;
    Texture2D<float4> textures[2];
}

ParameterBlock<Resources> resources;
static Texture2D<float4> cachedTexture = resources.texture;
static Texture2D<float4> cachedTextures[2] = resources.textures;
RWStructuredBuffer<float4> output;

[numthreads(1, 1, 1)]
void main()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0)) + cachedTextures[1].Load(int3(0, 0, 0));
}
