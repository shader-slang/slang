// The initializer for `cachedValue` calls `readCachedTexture` outside any function body. If the
// resource-global pass added a parameter to that helper, no function-local replacement value would
// exist for the initializer call to pass. We reject the helper before changing its signature.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage fragment

Texture2D<float4> inputTexture;
static Texture2D<float4> cachedTexture = inputTexture;

[noinline]
float4 readCachedTexture()
/*CHECK:
       ^^^^^^^^^^^^^^^^^ resource declared static at file or namespace scope is unavailable to a function whose invocations cannot all be rewritten
       ^^^^^^^^^^^^^^^^^ function 'readCachedTexture' may be invoked without a direct call that Slang can rewrite, so Slang cannot automatically pass the resource that it accesses; pass the resource explicitly instead
*/
{
    return cachedTexture.Load(int3(0, 0, 0));
}

static float4 cachedValue = readCachedTexture();

[shader("fragment")]
float4 main() : SV_Target
{
    return cachedValue;
}
