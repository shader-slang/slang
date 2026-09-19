//TEST:SIMPLE: -target hlsl -entry main -stage compute -o 13074-resource-initializer-read-only-state.hlsl

Texture2D<float4> textures[1];
RWStructuredBuffer<float> output;
static uint textureIndex = 0;

[noinline]
Texture2D<float4> initializeTexture(__constref uint index)
{
    return textures[index];
}

static Texture2D<float4> cachedTexture = initializeTexture(textureIndex);

export uint getTextureIndex()
{
    return textureIndex;
}

[numthreads(1, 1, 1)]
void main()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0)).x;
}
