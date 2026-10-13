//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -O0 -target hlsl -entry main -stage fragment -validate-ir -verify-debug-serial-ir

// The `initializeMain` entry point can write `cachedTexture`, so the module-wide
// uninitialized-global analysis cannot conclude that every read is uninitialized. The `main`
// entry point cannot reach that writer, however. Resource-global legalization must analyze `main`
// separately and diagnose the uninitialized value passed to `readCachedTexture`.

Texture2D<float4> textures[1];
static Texture2D<float4> cachedTexture;

[noinline]
void initializeCachedTexture()
{
    cachedTexture = textures[0];
}

[noinline]
float4 readCachedTexture()
{
    return cachedTexture.Load(int3(0, 0, 0));
}

[noinline]
float4 readCachedTextureThroughHelper()
{
    // We route the read through a second helper so that the test requires a transitive call-effect
    // summary rather than only the direct summary for `readCachedTexture`.
    return readCachedTexture();
}

[shader("fragment")]
float4 initializeMain() : SV_Target
{
    initializeCachedTexture();
    return float4(0.0);
}

[shader("fragment")]
float4 main() : SV_Target
{
    return readCachedTextureThroughHelper();
//CHECK:                                 ^ use of uninitialized variable
//CHECK:                                 ^ use of uninitialized variable 'cachedTexture'
}
