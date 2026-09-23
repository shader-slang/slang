//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -O0 -target hlsl -entry main -stage fragment -validate-ir

Texture2D<float4> textures[1];
static Texture2D<float4> cachedTextures[2];
//CHECK:                 ^^^^^^^^^^^^^^ cannot prove element-wise initialization of file-scope static resource array
//CHECK:                 ^^^^^^^^^^^^^^ file-scope static resource array 'cachedTextures' has individual element writes and may be read before a whole-array assignment; assign the whole array before reading it

// Resource-global legalization recognizes a whole-array assignment but does not combine assignments
// to individual elements. This diagnostic states that limitation instead of incorrectly claiming
// that an element below is definitely uninitialized.

[shader("fragment")]
float4 main() : SV_Target
{
    cachedTextures[0] = textures[0];
    return cachedTextures[1].Load(int3(0, 0, 0));
}
