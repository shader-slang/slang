//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage fragment

Texture2D<float4> textures[1];
uniform bool selectTexture;
static Texture2D<float4> selectedTexture;

[shader("fragment")]
float4 main() : SV_Target
{
    if (selectTexture)
        selectedTexture = textures[0];
    return selectedTexture.Load(int3(0, 0, 0));
//CHECK:                       ^ possible use of uninitialized variable
//CHECK:                       ^ variable 'selectedTexture' is not definitely initialized before this use
}
