//DIAGNOSTIC_TEST(dxc):SIMPLE(diag=CHECK): -O0 -target dxil -profile ps_6_0 -entry main -validate-ir

// Passing the selected resource through `__constref` requires its address. That use prevents SSA
// simplification from replacing the entry-point local with `textures[0]`, and D3D has no
// representation for the remaining addressable resource storage.

Texture2D<float4> textures[1];
static Texture2D<float4> selectedTexture;
//CHECK:                 ^^^^^^^^^^^^^^^ target cannot represent an opaque value in mutable local storage
//CHECK:                 ^^^^^^^^^^^^^^^ mutable local storage contains an opaque value of type 'Texture2D', which target 'dxil' cannot represent; use or pass the resource value directly instead of assigning it to a mutable local or `static` variable

[noinline]
float4 readExplicitly(__constref Texture2D<float4> explicitTexture)
{
    return explicitTexture.Load(int3(0, 0, 0));
}

[shader("fragment")]
float4 main() : SV_Target
{
    selectedTexture = textures[0];
    return readExplicitly(selectedTexture);
}
