// Moving a resource initializer must not move an unrelated ordinary initializer. In particular,
// `getUnrelatedValue` may be called without first invoking this module's shader entry point.
//
//TEST:SIMPLE: -target hlsl -entry main -stage compute -g -validate-ir -verify-debug-serial-ir -o 13074-resource-initializer-unrelated-state.hlsl

Texture2D<float4> texture;
RWStructuredBuffer<float> output;

[noinline]
Texture2D<float4> returnTexture(Texture2D<float4> value)
{
    return value;
}

static Texture2D<float4> cachedTexture = returnTexture(texture);
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
