//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -O0 -target hlsl -validate-ir

Texture2D<float4> textures[2];
static Texture2D<float4> cachedTexture = textures[0];

[shader("fragment")]
float4 callee() : SV_Target
/*CHECK:
       ^^^^^^ resource-dependent static state is not supported in a callable entry point
       ^^^^^^ entry point 'callee' is also called as an ordinary function and transitively accesses per-invocation resource-dependent static state; move the shared implementation to an unannotated helper
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
