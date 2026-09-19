//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -O0 -target hlsl -validate-ir

Texture2D<float4> texture;

[shader("fragment")]
float4 recordInitialization() : SV_Target
/*CHECK:
       ^^^^^^^^^^^^^^^^^^^^ resource-dependent static state is not supported in a callable entry point
       ^^^^^^^^^^^^^^^^^^^^ entry point 'recordInitialization' is also called as an ordinary function and transitively accesses per-invocation resource-dependent static state; move the shared implementation to an unannotated helper
*/
{
    static uint initializationCount = 0;
    return ++initializationCount;
}

[noinline]
Texture2D<float4> initializeTexture()
{
    recordInitialization();
    return texture;
}

static Texture2D<float4> cachedTexture = initializeTexture();

[shader("fragment")]
float4 main() : SV_Target
{
    return cachedTexture.Load(int3(0, 0, 0)) + recordInitialization();
}
