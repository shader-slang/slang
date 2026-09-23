// A generic-assembly terminator supplies the emitted implementation of its function. Ordinary
// instructions in the function body are not emitted around that implementation, so resource-global
// legalization must reject the function instead of adding copy-in or copy-out instructions that
// would not be present in the emitted generic-assembly implementation.
//
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry main -stage fragment

Texture2D<float4> inputTexture;
static Texture2D<float4> cachedTexture;

float4 loadCachedTexture()
/*CHECK:
       ^^^^^^^^^^^^^^^^^ resource declared static at file or namespace scope is unavailable to a generic-assembly function
       ^^^^^^^^^^^^^^^^^ function 'loadCachedTexture' uses generic assembly as its emitted implementation, so Slang cannot preserve the function body's access to resources declared static at file or namespace scope; pass the resource explicitly as an assembly operand instead
*/
{
    cachedTexture = inputTexture;
    __intrinsic_asm "$0.Load(int3(0, 0, 0))", cachedTexture;
}

[shader("fragment")]
float4 main() : SV_Target
{
    return loadCachedTexture();
}
