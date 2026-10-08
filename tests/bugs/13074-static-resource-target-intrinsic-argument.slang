//DIAGNOSTIC_TEST(dxc):SIMPLE(diag=CHECK): -O0 -target dxil -profile ps_6_0 -entry main -validate-ir

// The applicable target-intrinsic definition of `loadThroughIntrinsic` reads its resource argument,
// while the ordinary IR body deliberately returns a constant. The D3D resource-input inlining step
// must not inline that body merely to eliminate the resource argument, because doing so would
// silently discard the selected implementation and make the resource selection dead. The
// conditional assignment keeps the call's argument as a load from the generated entry-point local.
// Because the call remains, the late target check must diagnose that local instead of allowing
// invalid DXIL to reach the emitter.

Texture2D<float4> textures[2];
static Texture2D<float4> selectedTexture = textures[0];
//CHECK:                 ^^^^^^^^^^^^^^^ target cannot represent an opaque value in mutable local storage
//CHECK:                 ^^^^^^^^^^^^^^^ mutable local storage contains an opaque value of type 'Texture2D', which target 'dxil' cannot represent; use or pass the resource value directly instead of assigning it to a mutable local or `static` variable

__target_intrinsic(hlsl, "loadTextureIntrinsic($0)")
float4 loadThroughIntrinsic(Texture2D<float4> texture)
{
    return float4(0.0);
}

[shader("fragment")]
float4 main(uint textureIndex : INDEX) : SV_Target
{
    if ((textureIndex & 1) != 0)
        selectedTexture = textures[1];
    return loadThroughIntrinsic(selectedTexture);
}
