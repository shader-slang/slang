// The initializer for `cachedValue` executes outside a function, so no entry-point local can
// replace `cachedTexture` at that use. We reject the declaration instead of leaving a use of the
// global after legalization.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage fragment

Texture2D<float4> inputTexture;
static Texture2D<float4> cachedTexture = inputTexture;
static float4 cachedValue = cachedTexture.Load(int3(0, 0, 0));
//CHECK:                                      ^ mutable resource variable is used by executable code outside a function
//CHECK:                                      ^ mutable resource variable 'cachedTexture' is referenced by an operation outside a function body; perform that operation inside an entry point or another function

[shader("fragment")]
float4 main() : SV_Target
{
    return cachedValue;
}
