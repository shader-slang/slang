//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -O0 -target hlsl -entry main -stage fragment -validate-ir

Texture2D<float4> textures[1];
static Texture2D<float4> cachedTextures[2];

[shader("fragment")]
float4 main() : SV_Target
{
    cachedTextures[0] = textures[0];
    return cachedTextures[1].Load(int3(0, 0, 0));
//CHECK:                         ^ possible use of uninitialized variable
//CHECK:                         ^ variable 'cachedTextures' is not definitely initialized before this use
}
