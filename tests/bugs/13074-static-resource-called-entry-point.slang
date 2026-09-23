//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -O0 -target hlsl -validate-ir

Texture2D<float4> textures[2];
static Texture2D<float4> cachedTexture = textures[0];

[shader("fragment")]
float4 callee() : SV_Target
/*CHECK:
       ^^^^^^ resource declared static at file or namespace scope is not supported in a callable entry point
       ^^^^^^ entry point 'callee' may also be invoked as a callable function, and it accesses a resource declared static at file or namespace scope; Slang cannot use one function both as an entry point that creates the replacement local and as a callee that receives the replacement value; move the shared implementation to a separate function without an entry-point attribute
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
