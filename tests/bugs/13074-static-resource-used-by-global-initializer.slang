// The initializer for `cachedValue` executes outside a function, so no entry-point local can
// replace `cachedTexture` at that use. We reject the declaration instead of leaving a use of the
// global after legalization.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage fragment

Texture2D<float4> inputTexture;
static Texture2D<float4> cachedTexture = inputTexture;
static float4 cachedValue = cachedTexture.Load(int3(0, 0, 0));
//CHECK:                                      ^ file-scope static resource is used by another global initializer
//CHECK:                                      ^ file-scope static resource 'cachedTexture' is used while initializing another global variable; move that initialization into a function

[shader("fragment")]
float4 main() : SV_Target
{
    return cachedValue;
}
