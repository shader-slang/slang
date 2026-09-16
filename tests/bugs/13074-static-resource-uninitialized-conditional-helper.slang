//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -O0 -target hlsl -entry main -stage fragment -validate-ir

Texture2D<float4> textures[1];
uniform bool chooseTexture;
static Texture2D<float4> cachedTexture;

[noinline]
void maybeInitialize()
{
    if (chooseTexture)
        cachedTexture = textures[0];
}

[shader("fragment")]
float4 main() : SV_Target
{
    maybeInitialize();
    return cachedTexture.Load(int3(0, 0, 0));
//CHECK:                     ^ possible use of uninitialized variable
//CHECK:                     ^ variable 'cachedTexture' is not definitely initialized before this use
}
