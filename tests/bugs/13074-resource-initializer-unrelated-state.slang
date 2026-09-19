//TEST:SIMPLE: -target hlsl -entry main -stage compute -o 13074-resource-initializer-unrelated-state.hlsl

Texture2D<float4> texture;
RWStructuredBuffer<float> output;
static Texture2D<float4> cachedTexture = texture;
static uint unrelatedValue = 17;

export uint getUnrelatedValue()
{
    return unrelatedValue;
}

[numthreads(1, 1, 1)]
void main()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0)).x;
}
