//TEST:SIMPLE: -O0 -target hlsl -entry main -stage fragment -validate-ir -o 13074-static-resource-all-paths-write.hlsl

Texture2D<float4> textures[2];
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

[shader("fragment")]
float4 main() : SV_Target
{
    initialize();
    return cachedTexture.Load(int3(0, 0, 0));
}
