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
//CHECK:                 ^ resource declared static at file or namespace scope cannot be passed by reference and accessed by name when either path may modify it
//CHECK:                 ^ resource 'selectedTexture' is declared static at file or namespace scope and is passed by reference to function 'replaceAndRead', which also accesses the resource by name, and at least one of those paths may modify it; pass the resource through only one path
}
