//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -O0 -target hlsl -validate-ir

Texture2D<float4> textures[2];
static Texture2D<float4> cachedTexture = textures[0];

[shader("fragment")]
float4 callee() : SV_Target
/*CHECK:
       ^^^^^^ file-scope static resource is not supported in a callable entry point
       ^^^^^^ entry point 'callee' can also be invoked as an ordinary function and accesses a file-scope static resource; move the shared implementation to a separate function without an entry-point attribute
*/
{
    return cachedTexture.Load(int3(0, 0, 0));
}

[shader("fragment")]
float4 main() : SV_Target
{
    cachedTexture = textures[1];
    return callee();
}
