//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -O0 -target hlsl -entry main -stage fragment -validate-ir

Texture2D<float4> textures[1];
static Texture2D<float4> cachedTextures[2];
//CHECK:                 ^^^^^^^^^^^^^^ cannot prove complete initialization of a mutable resource array
//CHECK:                 ^^^^^^^^^^^^^^ mutable resource array 'cachedTextures' may be written one element at a time and may be read before a whole-array assignment; assign the whole array before reading it

// Resource-global legalization recognizes a whole-array assignment but does not combine
// assignments to individual elements. The diagnostic reports that the pass lacks an
// element-sensitive proof of complete initialization; it does not attempt to identify the
// uninitialized element.

[shader("fragment")]
float4 main() : SV_Target
{
    cachedTextures[0] = textures[0];
    return cachedTextures[1].Load(int3(0, 0, 0));
}
