// An applicable target-intrinsic definition replaces the fallback IR body. The fallback body
// therefore cannot prove how the emitted implementation uses an address passed to it.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage fragment

Texture2D<float4> inputTexture;
static Texture2D<float4> cachedTexture;

__target_intrinsic(hlsl, "useResourceAddress($0)")
void passToHLSLIntrinsic(inout Texture2D<float4> value)
{
}

[shader("fragment")]
float4 main() : SV_Target
{
    cachedTexture = inputTexture;
    passToHLSLIntrinsic(cachedTexture);
//CHECK:               ^ address use may observe the identity of storage for a resource declared static at file or namespace scope
//CHECK:               ^ the program may retain the address of resource 'cachedTexture', which is declared static at file or namespace scope, or observe its storage identity; replacing the variable with function-local storage might change that behavior
    return 0.0;
}
