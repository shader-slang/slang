// `selectThroughLocal` writes only its own local variables. No code outside the call can observe
// those stores, so moving the call to the start of each entry point preserves behavior.
//
//TEST:SIMPLE: -target hlsl -entry main -stage compute -O0 -validate-ir -o 13074-resource-initializer-local-write.hlsl

Texture2D<float4> firstTexture;
Texture2D<float4> secondTexture;
uniform uint selectedTexture;
RWStructuredBuffer<float4> output;

[noinline]
Texture2D<float4> selectThroughLocal(
    Texture2D<float4> first,
    Texture2D<float4> second,
    uint index)
{
    Texture2D<float4> firstLocal = first;
    Texture2D<float4> secondLocal = second;
    Ptr<Texture2D<float4>> selectedLocal = index == 0 ? &firstLocal : &secondLocal;
    return *selectedLocal;
}

static Texture2D<float4> cachedTexture =
    selectThroughLocal(firstTexture, secondTexture, selectedTexture);

[numthreads(1, 1, 1)]
void main()
{
    output[0] = cachedTexture.Load(int3(0, 0, 0));
}
