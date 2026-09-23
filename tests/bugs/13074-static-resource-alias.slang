//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage fragment

Texture2D<float4> textures[1];
static Texture2D<float4> selectedTexture = textures[0];

[noinline]
float4 replaceAndRead(inout Texture2D<float4> alias)
{
    alias = textures[0];
    return selectedTexture.Load(int3(0, 0, 0));
}

[shader("fragment")]
float4 main() : SV_Target
{
    return replaceAndRead(selectedTexture);
//CHECK:                 ^ file-scope static resource cannot be both passed by reference and accessed by name
//CHECK:                 ^ file-scope static resource 'selectedTexture' is passed by reference to function 'replaceAndRead', which also accesses the same variable by name; pass the resource through only one of those paths
}
