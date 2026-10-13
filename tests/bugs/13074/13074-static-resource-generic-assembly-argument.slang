// Generic assembly supplies the emitted implementation of `loadThroughAssembly`, so the D3D
// resource-input inlining step cannot inline its call. The argument remains loaded from the
// generated entry-point local. A separate `__constref` use keeps that local addressable, so the late
// target check must diagnose the storage instead of emitting invalid HLSL.

//DIAGNOSTIC_TEST(dxc):SIMPLE(diag=CHECK): -O0 -target dxil -profile ps_6_0 -entry main -validate-ir

Texture2D<float4> textures[2];
static Texture2D<float4> selectedTexture = textures[0];
//CHECK:                 ^^^^^^^^^^^^^^^ target cannot represent an opaque value in mutable local storage
//CHECK:                 ^^^^^^^^^^^^^^^ mutable local storage contains an opaque value of type 'Texture2D', which target 'dxil' cannot represent; use or pass the resource value directly instead of assigning it to a mutable local or `static` variable

[noinline]
float4 loadThroughAssembly(Texture2D<float4> texture)
{
    __intrinsic_asm "$0.Load(int3(0, 0, 0))", texture;
}

[noinline]
float4 loadThroughAddress(__constref Texture2D<float4> texture)
{
    return texture.Load(int3(0, 0, 0));
}

[shader("fragment")]
float4 main(uint textureIndex : INDEX) : SV_Target
{
    selectedTexture = textures[textureIndex & 1];
    return loadThroughAssembly(selectedTexture) + loadThroughAddress(selectedTexture);
}
