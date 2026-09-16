//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -O0 -target hlsl -validate-ir

Texture2D<float4> textures[2];
static Texture2D<float4> cachedTexture = textures[0];
//CHECK:                 ^^^^^^^^^^^^^ resource-dependent static state is not supported in a callable entry point
//CHECK:                 ^^^^^^^^^^^^^ entry point 'callee' is also called as an ordinary function and accesses static variable 'cachedTexture'; move the shared implementation to an unannotated helper

[shader("fragment")]
float4 callee() : SV_Target
{
    return cachedTexture.Load(int3(0, 0, 0));
}

[shader("fragment")]
float4 main() : SV_Target
{
    cachedTexture = textures[1];
    return callee();
}
