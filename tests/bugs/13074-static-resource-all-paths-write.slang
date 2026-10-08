//TEST:SIMPLE: -O0 -target cpp -entry computeMain -stage compute -validate-ir -o 13074-static-resource-all-paths-write.cpp

// This test checks definite initialization across both branches of a helper. The C++ target can
// represent the resulting local resource storage, so target restrictions do not mask that check.

Texture2D<float4> textures[2];
RWStructuredBuffer<float4> output;
uniform bool chooseTexture;
static Texture2D<float4> cachedTexture;

[noinline]
void initialize()
{
    if (chooseTexture)
        cachedTexture = textures[0];
    else
        cachedTexture = textures[1];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    initialize();
    output[0] = cachedTexture.Load(int3(0, 0, 0));
}
