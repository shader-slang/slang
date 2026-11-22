//TEST:SIMPLE: -O0 -target hlsl -entry main -stage compute -validate-ir -o 13074-static-resource-conditional-write-without-read.hlsl

Texture2D<float4> textures[1];
uniform bool chooseTexture;
static Texture2D<float4> cachedTexture;
RWStructuredBuffer<uint> output;

[noinline]
void maybeInitialize()
{
    if (chooseTexture)
        cachedTexture = textures[0];
}

[numthreads(1, 1, 1)]
void main()
{
    // `maybeInitialize` leaves `cachedTexture` uninitialized on one path, but this entry point
    // never reads it. The call alone must not produce an uninitialized-value diagnostic.
    maybeInitialize();
    output[0] = 1;
}
