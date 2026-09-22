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
    // We intentionally leave `cachedTexture` uninitialized on one path. Propagating that value
    // through hidden state is not a source-level read, so we diagnose only if later source code
    // observes it.
    maybeInitialize();
    output[0] = 1;
}
