//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage fragment

Texture2D<float4> textures[1];
static Texture2D<float4> selectedTexture;

[noinline]
float4 readExplicitlyAndByName(__constref Texture2D<float4> explicitTexture)
{
    return explicitTexture.Load(int3(0, 0, 0)) + selectedTexture.Load(int3(0, 0, 0));
}

[shader("fragment")]
float4 main() : SV_Target
{
    selectedTexture = textures[0];
    return readExplicitlyAndByName(selectedTexture);
//CHECK:                          ^ file-scope static resource cannot be accessed both by name and explicit reference
//CHECK:                          ^ file-scope static resource 'selectedTexture' is passed by reference to function 'readExplicitlyAndByName', which also accesses it by name; pass the resource through only one of those paths
}
