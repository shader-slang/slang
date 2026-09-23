// Loading a texture field from a parameter block copies the texture value. It does not read the
// texture's contents, so moving this initializer to the start of each entry point preserves its
// behavior.

//TEST:SIMPLE: -target hlsl -entry main -stage compute -validate-ir -o 13074-resource-initializer-parameter-block-field.hlsl

struct Resources
{
    Texture2D<float4> texture;
}

ParameterBlock<Resources> resources;
static Texture2D<float4> cachedTexture = resources.texture;
RWStructuredBuffer<float4> output;

[numthreads(1, 1, 1)]
void main()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0));
}
